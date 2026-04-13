#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include "promptly.h"

#define PROMPTLY_DEFAULT_COLS 80
#define PROMPTLY_DEFAULT_ROWS 24

#define SET_CTX_STATE(ctx, new_state) ctx->state = new_state;

#define PROMPTLY_WRITE(ctx, message, length) ctx->write(message, length);

/*
    The PROMPTLY_CONTINUE macro is a helper that simplifies the process of 
    continuing to edit the line after certain operations. It calls the 
    promptly_edit_line function with a null character, which allows the 
    state machine to continue processing without needing an actual character input.
*/
#define PROMPTLY_CONTINUE(ctx) promptly_edit_line(ctx, '\0');

typedef enum promptly_key {
    PROMPTLY_UNKNOWN = 0,
    PROMPTLY_BACKSPACE,
    PROMPTLY_ENTER,
    PROMPTLY_CHAR,
    PROMPTLY_EXTENDED, /* Used for arrow keys */
} promptly_key_t;

__attribute__((unused))
static void promptly_log(PROMPTLY_CTX, const char *format, ...) {
    promptly_hide(ctx);     /* Hide the current line before logging */

    va_list args;
    va_start(args, format);
    vprintf(format, args);  /* Log the message with variadic arguments */
    va_end(args);
    printf("\n");

    promptly_show(ctx);     /* Re-show the line after logging */
}

static promptly_key_t classify_key(char ch)
{
    switch (ch)
    {
        case '\b':
        case 127:
            return PROMPTLY_BACKSPACE;
        case '\n':
        case '\r':
            return PROMPTLY_ENTER;
        case (char)'\xE0':
            return PROMPTLY_EXTENDED;
    default:
        if (isprint((unsigned char)ch)) {
            return PROMPTLY_CHAR;
        }
        break;
    }

    return PROMPTLY_UNKNOWN;
}

__attribute__((unused))
static size_t get_cursor_position(PROMPTLY_CTX)
{
    /*
        Without interogating the terminal for the actual cursor position,
        we can estimate the cursor's column position based on the prompt length
        and the current write position within the line. This is a simplification
    */
    return ctx->prompt_length + ( ctx->line_wpos+1 );
}

static void move_cursor_left(PROMPTLY_CTX, size_t positions)
{
    if(positions == 0) {
        return;
    }
    if(positions == 1) {
        /* Optimization for single position move */
        PROMPTLY_WRITE(ctx, &(char){'\b'}, 1);
        return;
    }
    char move_back_seq[24];
    int r = snprintf(move_back_seq, sizeof(move_back_seq), "\x1b[%zuD", positions);
    if(r > 0) {
        PROMPTLY_WRITE(ctx, move_back_seq, (size_t)r);
    }
}

__attribute__((unused))
static void set_cursor_position_col(PROMPTLY_CTX, size_t col)
{
    char move_to_col_seq[24];
    int r = snprintf(move_to_col_seq, sizeof(move_to_col_seq), "\x1b[%zuG", col);
    if(r > 0) {
        PROMPTLY_WRITE(ctx, move_to_col_seq, (size_t)r);
    }
}

static void move_cursor_right(PROMPTLY_CTX, size_t positions)
{
    if(positions == 0) {
        return;
    }
    char move_forward_seq[24];
    int r = snprintf(move_forward_seq, sizeof(move_forward_seq), "\x1b[%zuC", positions);
    if(r > 0) {
        PROMPTLY_WRITE(ctx, move_forward_seq, (size_t)r);
    }
}

__attribute__((unused))
static void save_cursor_position(PROMPTLY_CTX)
{
    const char save_cursor_seq[] = "\033[s";  
    PROMPTLY_WRITE(ctx, save_cursor_seq, sizeof(save_cursor_seq) - 1);
}

__attribute__((unused))
static void restore_cursor_position(PROMPTLY_CTX)
{
    const char restore_cursor_seq[] = "\033[u";
    PROMPTLY_WRITE(ctx, restore_cursor_seq, sizeof(restore_cursor_seq) - 1);
}

static promptly_result_t parse_dsr_response(PROMPTLY_CTX, const char ch)
{
    if(ch=='R') /* End of DSR response */ {
        ctx->metadata[ctx->metadata_length] = '\0'; 
        if (sscanf(ctx->metadata, "\x1b[%zu;%zu", &ctx->rows, &ctx->cols) != 2) {
            ctx->cols = PROMPTLY_DEFAULT_COLS; 
            ctx->rows = PROMPTLY_DEFAULT_ROWS;
        }
        ctx->metadata_length = 0; 
        SET_CTX_STATE(ctx, PROMPTLY_SHOW_LINE);
        return PROMPTLY_CONTINUE(ctx); 
    }
    else {

        /* We use the metadata buffer to accumulate the DSR response. */
        if (ctx->metadata_length < sizeof(ctx->metadata) - 1) {
            ctx->metadata[ctx->metadata_length] = ch; 
            ctx->metadata_length++;
        }
        else {
            /* Metadata buffer overflow, reset state */
            ctx->metadata_length = 0;
            ctx->cols = PROMPTLY_DEFAULT_COLS; 
            ctx->rows = PROMPTLY_DEFAULT_ROWS;

            SET_CTX_STATE(ctx, PROMPTLY_SHOW_LINE);
            return PROMPTLY_CONTINUE(ctx);
        }
        return PROMPTLY_IDLE;
    }
}

static promptly_result_t parse_input(PROMPTLY_CTX, const char ch)
{
    const promptly_key_t key_type = classify_key(ch);
    switch (key_type) {
    case PROMPTLY_BACKSPACE: {
            if(ctx->line_wpos > 0) {
                if(ctx->line_wpos < ctx->line_size) {
                    /* Deleting in the middle or beginning of the line */
                    memmove(&ctx->line[ctx->line_wpos - 1], 
                            &ctx->line[ctx->line_wpos], 
                            ctx->line_size - ctx->line_wpos);

                    ctx->line_wpos--;
                    ctx->line_size--;

                    /* Re-writed the changes to terminal */
                    move_cursor_left(ctx, 1);     
                    PROMPTLY_WRITE(ctx, &ctx->line[ctx->line_wpos], ctx->line_size - ctx->line_wpos);
                    PROMPTLY_WRITE(ctx, " ", 1);            
                    move_cursor_left(ctx, ctx->line_size - ctx->line_wpos + 1);
                }
                else {
                    /* Deleting at the end of the line */
                    ctx->line_wpos--;
                    ctx->line_size--;
                    move_cursor_left(ctx, 1);
                    PROMPTLY_WRITE(ctx, " ", 1);            
                    move_cursor_left(ctx, 1);
                }
            }
            else {
                promptly_bell(ctx);
            }
    }
    break;

    case PROMPTLY_ENTER: {
            ctx->line[ctx->line_size] = '\0';
            PROMPTLY_WRITE(ctx, "\n", 1);
            return PROMPTLY_END_LINE;
    }
    break;

    case PROMPTLY_CHAR: {
            if(ctx->line_size <= ctx->line_length-1) {
                if(ctx->line_wpos < ctx->line_size) {
                    /* Inserting in the middle or beginning of the line */
                    size_t to_shift = ctx->line_size - ctx->line_wpos;

                    /* Shift left*/
                    memmove(&ctx->line[ctx->line_wpos + 1], 
                            &ctx->line[ctx->line_wpos], 
                            to_shift);
                            
                    ctx->line[ctx->line_wpos] = ch; 
                    PROMPTLY_WRITE(ctx, &ctx->line[ctx->line_wpos], to_shift + 1);

                    ctx->line_wpos++;
                    ctx->line_size++;
                    
                    move_cursor_left(ctx, ctx->line_size - ctx->line_wpos);
                }
                else {
                    /* Inserting at the end of the line */
                    ctx->line[ctx->line_wpos] = ch; 
                    ctx->line_wpos++;
                    ctx->line_size++;
                    PROMPTLY_WRITE(ctx, &ch, 1);
                }
            }
            else {
                promptly_bell(ctx);             
            }
    }
    break;

    case PROMPTLY_EXTENDED: {
        SET_CTX_STATE(ctx, PROMPTLY_PARSE_EXTENDED);
        break;  
    }

    default:
        break;
    }

    return PROMPTLY_IDLE;
}

static promptly_result_t parse_extended(PROMPTLY_CTX, const char ch) 
{
    enum { LEFT_ARROW = 'K', RIGHT_ARROW = 'M', UP_ARROW = 'H', DOWN_ARROW = 'P' };
        
    switch (ch) {
        case LEFT_ARROW: {
            // Allow moving left only if we're not at the beginning of the line
            if(ctx->line_wpos > 0) {
                ctx->line_wpos--;
                move_cursor_left(ctx, 1);
            }
            else {
                promptly_bell(ctx); 
            }
        }
        break;
        case RIGHT_ARROW:
            if(ctx->line_wpos < ctx->line_size) {
                ctx->line_wpos++;
                move_cursor_right(ctx, 1);
            }
            else {
                promptly_bell(ctx); 
            }
            /* Handle right arrow key */
            break;
        case UP_ARROW:
            /* Handle up arrow key */
            break;
        case DOWN_ARROW:
            /* Handle down arrow key */
            break;
        default:
            break;
    }

    SET_CTX_STATE(ctx, PROMPTLY_PARSE_INPUT);
    return PROMPTLY_IDLE;
}

promptly_result_t promptly_edit_line(PROMPTLY_CTX, const char ch)
{
    if(ctx == NULL) {
        return PROMPTLY_ERROR;
    }

    switch (ctx->state) {
    case PROMPTLY_NONE: {
        SET_CTX_STATE(ctx, PROMPTLY_REQ_DSR);
        return PROMPTLY_CONTINUE(ctx);
    }
    
    case PROMPTLY_REQ_DSR: {
        PROMPTLY_WRITE(ctx, "\x1b[999;999H", 11);
        PROMPTLY_WRITE(ctx, "\x1b[6n", 4);
        SET_CTX_STATE(ctx, PROMPTLY_PARSE_DSR);
        return PROMPTLY_IDLE;
    }

    case PROMPTLY_PARSE_DSR: {
        return parse_dsr_response(ctx, ch);   
    }

    case PROMPTLY_SHOW_LINE: {
        promptly_show_line(ctx);
        SET_CTX_STATE(ctx, PROMPTLY_PARSE_INPUT);
        return PROMPTLY_IDLE;
    }

    case PROMPTLY_PARSE_INPUT: {
        return parse_input(ctx, ch);
    }

    case PROMPTLY_PARSE_EXTENDED: {
        return parse_extended(ctx, ch);
    }

    default:
        return PROMPTLY_ERROR;
    }
}

promptly_result_t promptly_start_line(PROMPTLY_CTX)
{
    if(ctx == NULL) {
        return PROMPTLY_ERROR;
    }

    ctx->line_size = 0;     
    ctx->line_wpos = 0;     
    ctx->line[0] = '\0';
    SET_CTX_STATE(ctx, PROMPTLY_NONE);
    
    return PROMPTLY_CONTINUE(ctx);
}

void promptly_show_line(PROMPTLY_CTX)
{
    PROMPTLY_WRITE(ctx, ctx->prompt, strlen(ctx->prompt));
    PROMPTLY_WRITE(ctx, ctx->line, ctx->line_size);
}

void promptly_bell(PROMPTLY_CTX)
{
    const char bell_seq[] = "\a";
    PROMPTLY_WRITE(ctx, bell_seq, sizeof(bell_seq) - 1);
}

__attribute__((unused))
void promptly_hide(PROMPTLY_CTX)
{
    ctx->hcpos = get_cursor_position(ctx); /* Save current cursor position */
    /* Move cursor to the beginning of the line */
    set_cursor_position_col(ctx, 1);

    /* Clear the line from the cursor to the end */
    const char clear_line_seq[] = "\x1b[K";
    PROMPTLY_WRITE(ctx, clear_line_seq, sizeof(clear_line_seq) - 1);
}

__attribute__((unused))
void promptly_show(PROMPTLY_CTX)
{
    /* Move cursor to the beginning of the line */
    set_cursor_position_col(ctx, 1);

    /* Clear the line from the cursor to the end */
    const char clear_line_seq[] = "\x1b[K";
    PROMPTLY_WRITE(ctx, clear_line_seq, sizeof(clear_line_seq) - 1);

    /* Re-show the prompt and line */
    promptly_show_line(ctx);
    set_cursor_position_col(ctx, ctx->hcpos);
}

void promptly_greet(void)
{
    printf("Welcome to Promptly!\n");
}

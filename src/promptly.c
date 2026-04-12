#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "promptly.h"

#define PROMTLY_DEFAULT_COLS 80
#define PROMTLY_DEFAULT_ROWS 24

#define SET_CTX_STATE(ctx, new_state) ctx->state = new_state;

#define PROMPTLY_WRITE(ctx, message, length) ctx->write(message, length);

#define PROMPTLY_WRITE_STR(ctx, message) PROMPTLY_WRITE(ctx, message, strlen(message)) 

#define PROMTLY_CONTINUE(ctx) promtly_edit_line(ctx, &(char){'\0'});

typedef enum promtly_key {
    PROMTLY_UNKNOWN = 0,
    PROMTLY_BACKSPACE,
    PROMTLY_ENTER,
    PROMTLY_CHAR,
    PROMTLY_EXTENDED, /* Used for arrow keys */
} promtly_key_t;

static promtly_key_t classify_key(char ch) {
    if (ch == '\b' || ch == 127) {
        return PROMTLY_BACKSPACE;
    } else if (ch == '\n' || ch == '\r') {
        return PROMTLY_ENTER;
    } else if (isprint((unsigned char)ch)) {
        return PROMTLY_CHAR;
    } else if (ch == (char)'\xE0') {
        return PROMTLY_EXTENDED;
    }
    return PROMTLY_UNKNOWN;
}

static void move_cursor_left(PROMTLY_CTX, size_t positions) {
    if(positions == 0) return; /* No need to move */
    if(positions == 1) {
        /* Optimization for single position move */
        PROMPTLY_WRITE(ctx, &(char){'\b'}, 1);
        return;
    }
    char move_back_seq[24];
    snprintf(move_back_seq, sizeof(move_back_seq), "\x1b[%zuD", positions);
    PROMPTLY_WRITE_STR(ctx, move_back_seq);
}

__attribute__((unused))
static void move_cursor_right(PROMTLY_CTX, size_t positions) {
    if(positions == 0) return; /* No need to move */
    char move_forward_seq[24];
    snprintf(move_forward_seq, sizeof(move_forward_seq), "\x1b[%zuC", positions);
    PROMPTLY_WRITE_STR(ctx, move_forward_seq);
}

__attribute__((unused))
static void save_cursor_position(PROMTLY_CTX) {
    const char save_cursor_seq[] = "\033[s";  
    PROMPTLY_WRITE(ctx, save_cursor_seq, sizeof(save_cursor_seq) - 1);
}

__attribute__((unused))
static void restore_cursor_position(PROMTLY_CTX) {
    const char restore_cursor_seq[] = "\033[u";
    PROMPTLY_WRITE(ctx, restore_cursor_seq, sizeof(restore_cursor_seq) - 1);
}

promtly_result_t promtly_edit_line(PROMTLY_CTX, char* ch) {
    if(ctx == NULL) {
        return PROMTLY_ERROR;
    }

    if(ch == NULL)
    {
        return PROMTLY_IDLE;
    }

    switch (ctx->state)
    {
    case PROMTLY_NONE: {
        SET_CTX_STATE(ctx, PROMTLY_REQ_DSR);
        return PROMTLY_CONTINUE(ctx);
    }
    
    case PROMTLY_REQ_DSR: {
        PROMPTLY_WRITE_STR(ctx, "\x1b[6n"); /* Request Device Status Report */
        SET_CTX_STATE(ctx, PROMTLY_PARSE_DSR);
        return PROMTLY_IDLE;
    }

    case PROMTLY_PARSE_DSR: {
        if(*ch=='R')
        {
            ctx->metadata[ctx->metadata_length] = '\0'; /* Null-terminate the metadata */
            if (sscanf(ctx->metadata, "\x1b[%zu;%zu", &ctx->rows, &ctx->cols) != 2) {
                ctx->cols = PROMTLY_DEFAULT_COLS; /* Fallback to default if parsing fails */
                ctx->rows = PROMTLY_DEFAULT_ROWS;
            }
            ctx->metadata_length = 0; /* Reset metadata length for future use */
            SET_CTX_STATE(ctx, PROMTLY_SHOW_LINE);
            return PROMTLY_CONTINUE(ctx); /* Trigger the next state immediately */
        }
        else
        {
            if (ctx->metadata_length < sizeof(ctx->metadata) - 1) {
                ctx->metadata[ctx->metadata_length] = *ch; /* Store metadata characters */
                ctx->metadata_length++;
            }
            else {
                /* Metadata buffer overflow, reset state */
                ctx->metadata_length = 0;
                SET_CTX_STATE(ctx, PROMTLY_SHOW_LINE);
                return PROMTLY_CONTINUE(ctx); /* Trigger the next state immediately */
            }
            return PROMTLY_IDLE;
        }
    }

    case PROMTLY_SHOW_LINE: {
        promtly_show_line(ctx);
        SET_CTX_STATE(ctx, PROMTLY_PARSE_INPUT);
        return PROMTLY_IDLE;
    }

    case PROMTLY_PARSE_INPUT: {
        const promtly_key_t key_type = classify_key(*ch);
        switch (key_type)
        {
        case PROMTLY_BACKSPACE: {
                if(ctx->line_wpos > 0) {
                    if(ctx->line_wpos < ctx->line_size)
                    {
                        /* Deleting in the middle or beginning of the line */
                        memmove(&ctx->line[ctx->line_wpos - 1], 
                                &ctx->line[ctx->line_wpos], 
                                ctx->line_size - ctx->line_wpos);

                        ctx->line_wpos--;
                        ctx->line_size--;

                        /* Re-writed the changes to terminal */
                        PROMPTLY_WRITE_STR(ctx, "\b");                 
                        PROMPTLY_WRITE(ctx, &ctx->line[ctx->line_wpos], ctx->line_size - ctx->line_wpos);
                        PROMPTLY_WRITE_STR(ctx, " ");            
                        
                        move_cursor_left(ctx, ctx->line_size - ctx->line_wpos + 1);
                    }
                    else
                    {
                        ctx->line_wpos--;
                        ctx->line_size--;
                        PROMPTLY_WRITE_STR(ctx, "\b \b");                 
                    }
                }
        }
        break;

        case PROMTLY_ENTER: {
                ctx->line[ctx->line_size] = '\0'; /* Null-terminate the line */
                return PROMTLY_END_LINE;
        }
        break;

        case PROMTLY_CHAR: {
                /* Leave space for null terminator */
                if(ctx->line_size <= ctx->line_length-1) {
                    if(ctx->line_wpos < ctx->line_size) {
                        /* Inserting in the middle or beginning of the line */
                       
                        size_t to_shift = ctx->line_size - ctx->line_wpos;

                        /* Shift left*/
                        memmove(&ctx->line[ctx->line_wpos + 1], 
                                &ctx->line[ctx->line_wpos], 
                                to_shift);
                                
                        ctx->line[ctx->line_wpos] = *ch; 
                        PROMPTLY_WRITE(ctx, &ctx->line[ctx->line_wpos], to_shift + 1);

                        ctx->line_wpos++;
                        ctx->line_size++;
                        
                        move_cursor_left(ctx, ctx->line_size - ctx->line_wpos);
                    }
                    else
                    {
                        ctx->line[ctx->line_wpos] = *ch; 
                        ctx->line_wpos++;
                        ctx->line_size++;
                        PROMPTLY_WRITE(ctx, ch, 1);
                    }
                }
        }
        break;

        case PROMTLY_EXTENDED: {
           SET_CTX_STATE(ctx, PROMTLY_PARSE_EXTENDED);
           break;  
        }

        default:
            break;
        }

        return PROMTLY_IDLE;
    }

    case PROMTLY_PARSE_EXTENDED: {
        enum { LEFT_ARROW = 'K', RIGHT_ARROW = 'M', UP_ARROW = 'H', DOWN_ARROW = 'P' };
        
        switch (*ch) {
            case LEFT_ARROW:
            {
                // Allow moving left only if we're not at the beginning of the line
                if(ctx->line_wpos > 0) {
                    ctx->line_wpos--;
                    PROMPTLY_WRITE_STR(ctx, "\b"); /* Move cursor left */
                }
            }
            break;
            case RIGHT_ARROW:
                if(ctx->line_wpos < ctx->line_size) {
                    ctx->line_wpos++;
                    PROMPTLY_WRITE_STR(ctx, "\x1b[C"); /* Move cursor right */
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

        SET_CTX_STATE(ctx, PROMTLY_PARSE_INPUT);
        return PROMTLY_IDLE;
    }

    default:
        break;
    }

    return PROMTLY_ERROR;
}

promtly_result_t promtly_start_line(PROMTLY_CTX)
{
    if(ctx == NULL) {
        return PROMTLY_ERROR;
    }

    ctx->line_size = 0; /* Reset line index for new input */
    ctx->line_wpos = 0; /* Reset write position */
    ctx->line[0] = '\0'; /* Clear the line buffer */
    SET_CTX_STATE(ctx, PROMTLY_NONE);
    
    return PROMTLY_CONTINUE(ctx);
}

void promtly_show_line(PROMTLY_CTX) {
    PROMPTLY_WRITE_STR(ctx, "\x1b[1G");
    PROMPTLY_WRITE_STR(ctx, ctx->prompt);
    PROMPTLY_WRITE(ctx, ctx->line, ctx->line_size);
}

void promptly_greet(void) {
    printf("Welcome to Promptly!\n");
}

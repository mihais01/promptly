#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "promptly.h"

#define PROMTLY_DEFAULT_COLS 80
#define PROMTLY_DEFAULT_ROWS 24

#define SET_CTX_STATE(ctx, new_state) ctx->state = new_state;

#define PROMPTLY_WRITE(ctx, message, length) ctx->write(message, length);

#define PROMTLY_WRITE_STR(ctx, message) PROMPTLY_WRITE(ctx, message, strlen(message)) 

#define PROMTLY_CONTINUE(ctx) promtly_edit_line(ctx, &(char){'\0'});

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
        PROMTLY_WRITE_STR(ctx, "\x1b[6n"); /* Request Device Status Report */
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
        if(*ch == '\b' || *ch == 127) { /* Handle backspace */
            if(ctx->line_size > 0) {
                ctx->line_wpos--;
                ctx->line_size--;
                PROMTLY_WRITE_STR(ctx, "\b \b"); /* Move cursor back, overwrite with space, move back again */
            }
        }
        else
        {
            if(ctx->line_size < ctx->line_length - 1) {
                if(*ch == '\n'|| *ch == '\r') {
                    ctx->line[ctx->line_size] = '\0'; /* Null-terminate the line */
                    return PROMTLY_END_LINE;
                }
                else {
                    ctx->line[ctx->line_wpos] = *ch; /* Store the input character */
                    ctx->line_wpos++;
                    ctx->line_size++;
                    PROMPTLY_WRITE(ctx, ch, 1); /* Echo the character for demonstration */
                }
            }
        }
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
    PROMTLY_WRITE_STR(ctx, "\x1b[1G");
    PROMTLY_WRITE_STR(ctx, ctx->prompt);
    PROMPTLY_WRITE(ctx, ctx->line, ctx->line_size);
}

void promptly_greet(void) {
    printf("Welcome to Promptly!\n");
}

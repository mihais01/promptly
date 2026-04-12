#ifndef PROMPTLY_H
#define PROMPTLY_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define PROMTLY_CTX struct promtly_ctx *const ctx

enum promptly_ctx_state {
    PROMTLY_NONE = 0,
    PROMTLY_REQ_DSR,       /* Request Device Status Report (for cursor pos)*/
    PROMTLY_PARSE_DSR, 
    PROMTLY_SHOW_LINE,
    PROMTLY_PARSE_INPUT,
    PROMTLY_PARSE_EXTENDED,
};

typedef enum promtly_result {
    PROMTLY_IDLE,
    PROMTLY_END_LINE,
    PROMTLY_ERROR,
} promtly_result_t;

struct promtly_cursor {
    size_t row;
    size_t col;
};

struct promtly_ctx {
    /* ==== Public configuration ==== */
    const char *prompt;
    size_t prompt_length;

    char* line;
    size_t line_length;
    size_t line_size;
    size_t line_wpos;

    void (*write)(const char *message, size_t length);

    /* ==== Private Metadata ==== */
    size_t cols;
    size_t rows; 

    struct promtly_cursor cursor;
    enum promptly_ctx_state state;

    char metadata[32];
    uint8_t metadata_length;
};

promtly_result_t promtly_edit_line(PROMTLY_CTX, char* ch);

promtly_result_t promtly_start_line(PROMTLY_CTX);

void promtly_show_line(PROMTLY_CTX);

void promptly_bell(PROMTLY_CTX);

void promptly_greet(void);

#endif

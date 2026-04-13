#ifndef PROMPTLY_H
#define PROMPTLY_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define PROMPTLY_CTX struct promptly_ctx *const ctx

enum promptly_ctx_state {
    PROMPTLY_NONE = 0,
    PROMPTLY_REQ_DSR,       /* Request Device Status Report (for cursor pos)*/
    PROMPTLY_PARSE_DSR, 
    PROMPTLY_SHOW_LINE,
    PROMPTLY_PARSE_INPUT,
    PROMPTLY_PARSE_EXTENDED,
};

typedef enum promptly_result {
    PROMPTLY_IDLE,
    PROMPTLY_END_LINE,
    PROMPTLY_ERROR,
} promptly_result_t;

struct promptly_ctx {
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

    enum promptly_ctx_state state;

    char metadata[32];
    uint8_t metadata_length;

    size_t hcpos; /* Saved cursor position during hide */
};

promptly_result_t promptly_edit_line(PROMPTLY_CTX, const char ch);

promptly_result_t promptly_start_line(PROMPTLY_CTX);

void promptly_show_line(PROMPTLY_CTX);

void promptly_bell(PROMPTLY_CTX);

void promptly_hide(PROMPTLY_CTX);

void promptly_show(PROMPTLY_CTX);

void promptly_greet(void);

#endif

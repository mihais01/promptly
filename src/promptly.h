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
    PROMTLY_PRINT_PROMPT,
    PROMTLY_PARSE_INPUT,

};

enum promtly_result {
    PROMTLY_EDIT = 0,
    PROMTLY_IDLE,
    PROMTLY_ERROR,
};

struct promtly_ctx {
    /* ==== Public configuration ==== */
    const char *prompt;
    size_t prompt_length;
    
    void (*write)(const char *message, size_t length);

    /* ==== Private Metadata ==== */
    size_t cols;
    size_t rows; 

    enum promptly_ctx_state state;

    /* Used for storing temporary metadata */
    char metadata[32];
    uint8_t metadata_length;
};

enum promtly_result promtly_edit_line(PROMTLY_CTX, char* ch);

void promtly_show_prompt(PROMTLY_CTX);

void promptly_greet(void);

#endif

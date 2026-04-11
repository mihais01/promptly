#ifndef PROMPTLY_H
#define PROMPTLY_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define PROMTLY_CTX struct promtly_ctx *const ctx

enum promptly_ctx_state {
    PROMTLY_NONE = 0,
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
};

void promtly_edit_line(PROMTLY_CTX, char ch);

void promptly_greet(void);

#endif

#ifndef PROMPTLY_H
#define PROMPTLY_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define PROMTLY_CTX struct promtly_ctx *const ctx

struct promtly_ctx {
    /* ==== Public configuration ==== */
    const char *prompt;
    size_t prompt_length;
    
    void (*write)(const char *message, size_t length);

    /* ==== Private Metadata ==== */
    size_t cols;
    size_t rows; 

};

void promtly_edit_line(PROMTLY_CTX);

void promptly_greet(void);

#endif

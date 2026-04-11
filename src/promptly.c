#include <stdio.h>
#include <string.h>

#include "promptly.h"

#define PROMTLY_DEFAULT_COLS 80
#define PROMTLY_DEFAULT_ROWS 24

#define PROMPTLY_WRITE(ctx, message) \
            ctx->write(message, strlen(message));


void promtly_edit_line(PROMTLY_CTX) {
    if(ctx == NULL) {
        return;
    }
}

void promptly_greet(void) {
    printf("Welcome to Promptly!\n");
}

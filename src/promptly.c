#include <stdio.h>
#include <string.h>

#include "promptly.h"

#define PROMTLY_DEFAULT_COLS 80
#define PROMTLY_DEFAULT_ROWS 24

#define PROMPTLY_WRITE(interface, message) \
    do { \
        if (g_config.write) { \
            g_config.write(interface, message, strlen(message)); \
        } \
    } while (0)

#define PROMPTLY_READ(interface, buffer, length) \
    (g_config.read ? g_config.read(interface, buffer, length) : -1)

#define PROMTLY_GET_CH(interface, ch) PROMPTLY_READ(interface, ch, 1)

static bool g_initialized = false;
static struct promptly_config g_config = {
    .write = NULL,
    .read = NULL
};

void promptly_init(const struct promptly_config *const config) {
    if (config == NULL) {
        return;
    }

    g_config = *config;
    g_initialized = true;
}

static void get_terminal_dimensions(PROMTLY_CTX) {
    /* Move cursor to bottom-right corner */
    PROMPTLY_WRITE(ctx->interface, "\x1b[999;999H"); 

    /* Request cursor position */
    PROMPTLY_WRITE(ctx->interface, "\x1b[6n");
    
    char response[32] = {0};

    size_t i = 0;
    for(;;) {

        if(i >= sizeof(response) - 1) {
            break; /* Prevent buffer overflow */
        }

        if(PROMTLY_GET_CH(ctx->interface, &response[i]) != 0) {
            if (response[i] == 'R') {
                response[i+1] = '\0';
                break;
            }
            else
            {
                i++;
            }
        }
    }

    /* Parse the response to get terminal dimensions */
    if (sscanf(response, "\x1b[%zu;%zuR", &ctx->rows, &ctx->cols) != 2) {
        ctx->cols = PROMTLY_DEFAULT_COLS; /* Fallback to default if parsing fails */
        ctx->rows = PROMTLY_DEFAULT_ROWS;
    }

    printf("\rTerminal dimensions: %zu cols, %zu rows\n", ctx->cols, ctx->rows);
}

void promtly_start(PROMTLY_CTX) {
    if (!g_initialized || ctx == NULL) {
        return;
    }

    /*
        Get Terminal dimensions.
    */
    get_terminal_dimensions( ctx );

}

void promtly_edit_line(PROMTLY_CTX, 
                            char *const buffer, 
                            size_t *const buffer_size) {
    (void) buffer; /* Unused parameter */
    (void) buffer_size; /* Unused parameter */
    (void) ctx; /* Unused parameter */
    
    if(!g_initialized || ctx == NULL) {
        return;
    }

    /* Move cursor to the beginning of the line */
    PROMPTLY_WRITE(ctx->interface, "\x1b[1G");
    PROMPTLY_WRITE(ctx->interface, ctx->prompt);
}

void promptly_greet(void) {
    if (!g_initialized) {
        return;
    }
    const char *greeting = "Hello from Promptly!\n";
    PROMPTLY_WRITE(0, greeting);
}

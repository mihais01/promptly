#include <stdio.h>
#include "promptly.h"

static bool g_initialized = false;
static struct promptly_config g_config = {
    .write = NULL,
    .read = NULL
};

void promptly_init(const struct promptly_config *config) {
    if (config == NULL) {
        return;
    }

    g_config = *config;
    g_initialized = true;
}

void promptly_greet(void) {
    if (!g_initialized) {
        return;
    }

    printf("Hello, Promptly\n");
}

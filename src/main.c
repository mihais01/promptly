#include "promptly.h"
#include <stdio.h>
#include <conio.h>

static void promtply_write(promptly_interface_t interface, const char *message, size_t length) {
    (void) interface; /* Unused parameter */
    (void) message;   /* Unused parameter */
    (void) length;    /* Unused parameter */


    /* For demonstration purposes, we'll just write to stdout regardless of the interface */
    fwrite(message, sizeof(char), length, stdout);
    fflush(stdout);
}

static ssize_t promtply_read(promptly_interface_t interface, char *buffer, size_t length) {
    (void) interface; /* Unused parameter */
    (void) buffer;    /* Unused parameter */
    (void) length;    /* Unused parameter */
    size_t i = 0;
    for (; i < length; i++)
    {
        if(!_kbhit()) {
            break;
        }

        buffer[i] = (char)_getch();
    }

    return (ssize_t)i;
}

int main(void)
{
    struct promptly_config config = {
        .write = promtply_write,
        .read = promtply_read
    };

    promptly_init(&config);
    promptly_greet();

    struct promtly_ctx ctx = {
        .interface = 0, /* Example interface */
        .prompt = ">>> ",
        .prompt_length = 4
    };
    promtly_start(&ctx);
    promtly_edit_line(&ctx, NULL, NULL);

    while(1){};

    return 0;
}

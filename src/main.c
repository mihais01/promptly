#include "promptly.h"
#include <stdio.h>
#include <conio.h>

static void promtply_write(const char *message, size_t length) {
    (void) message;   /* Unused parameter */
    (void) length;    /* Unused parameter */


    /* For demonstration purposes, we'll just write to stdout regardless of the interface */
    fwrite(message, sizeof(char), length, stdout);
    fflush(stdout);
}

/*
static ssize_t promtply_read(char *buffer, size_t length) {
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
*/

int main(void)
{
    struct promtly_ctx ctx = {
        .prompt = ">>> ",
        .prompt_length = 4,
        .write = promtply_write,
    };

    for(;;)
    {
        char ch = (char)_getch();
        promtly_edit_line(&ctx, ch);
    }    

    while(1){};

    return 0;
}

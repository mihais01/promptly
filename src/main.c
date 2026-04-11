#include "promptly.h"
#include <stdio.h>
#include <conio.h>

static void promtply_write(const char *message, size_t length) {
    (void) message;   /* Unused parameter */
    (void) length;    /* Unused parameter */

    for (size_t i = 0; i < length; i++)
    {
        fputc(message[i], stdout);
    }
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

        .state = PROMTLY_NONE,
    };

    for(;;)
    {
        if(_kbhit()) 
        {
            promtly_edit_line(&ctx, &(char){(char)_getch()});
        }
    }    

    return 0;
}

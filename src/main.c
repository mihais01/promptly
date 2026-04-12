#include "promptly.h"
#include <stdio.h>
#include <conio.h>

static void promptly_write(const char *message, size_t length) {
    (void) message;   /* Unused parameter */
    (void) length;    /* Unused parameter */

    fwrite(message, 1, length, stdout);
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
    char line[32] = {'\0'};

    struct promtly_ctx ctx = {
        .prompt = ">>> ",
        .prompt_length = 4,
        .write = promptly_write,

        .line = line,
        .line_length = sizeof(line),
        .line_i = 5,

        /* Initialize private metadata */
        .state = PROMTLY_NONE,
    };

    for(;;)
    {
        promtly_start_line(&ctx);

        while(1)
        {
            if(_kbhit()) 
            {
               if( promtly_edit_line(&ctx, &(char){(char)_getch()}) == PROMTLY_END_LINE) {
                   printf("\nYou entered: %s\n", ctx.line);
                   break;
               }
            }
        }
    }

    return 0;
}

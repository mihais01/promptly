#include "promptly.h"
#include <stdio.h>
#include <conio.h>

#include <windows.h>

static void promptly_write(const char *message, size_t length) {
    (void) message;   /* Unused parameter */
    (void) length;    /* Unused parameter */

    fwrite(message, 1, length, stdout);
}

int main(void)
{
    char line[32] = {'\0'};

    struct promptly_ctx ctx = {
        .prompt = ">>> ",
        .prompt_length = 4,
        .write = promptly_write,

        .line = line,
        .line_length = sizeof(line),
        .line_size = 5,

        /* Initialize private metadata */
        .state = PROMPTLY_NONE,
    };

    for(;;)
    {
        promptly_start_line(&ctx);

        while(1)
        {
            if(_kbhit()) 
            {
               if( promptly_edit_line(&ctx, &(char){(char)_getch()}) == PROMPTLY_END_LINE) {
                   printf("\nYou entered: %s\n", ctx.line);
                   break;
               }
            }
            else
            {
                Sleep(10); /* Sleep briefly to avoid busy-waiting */
            }
        }
    }

    return 0;
}

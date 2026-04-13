#include "promptly.h"
#include <stdio.h>
#include <conio.h>

#include <windows.h>

/*
    Example usage of Promptly library. 
    This example demonstrates how to use the Promptly library to create a simple 
    command-line interface that reads user input and echoes it back to the console. 
    
    The program continuously prompts the user for input until they choose to exit.
*/


/*
    Custom write function for Promptly context. This function is responsible for 
    writing output to the console. In this example, it simply writes the provided 
    message to standard output using fwrite. The length parameter indicates how many 
    bytes to write from the message buffer.
*/
static void promptly_write(const char *message, size_t length) {
    (void) message;   /* Unused parameter */
    (void) length;    /* Unused parameter */

    fwrite(message, 1, length, stdout);
}

int main(void)
{
    /* Buffer to hold the user input line. The size is set to 32 characters, which 
       includes the null terminator. This buffer will be used by the Promptly 
       context to store the current line being edited by the user. 
    */
    char line[1024];

    struct promptly_ctx ctx = {
        .prompt = ">>> ",
        .prompt_length = 4,
        .write = promptly_write,

        .line = line,
        .line_length = sizeof(line),
    };

    for(;;)
    {
        /*
            Start a new line input session. This will reset the line buffer and 
            prepare the context for new input. The user will see the prompt ">>> " 
            and can start typing their input. The program will continue to read 
            characters until the user presses Enter, at which point it will echo 
            the entered line back to the console.
        */
        promptly_start_line(&ctx);

        while(1)
        {
            if(_kbhit()) /* Check if a key has been pressed */
            {
                /* Character pressed by the user */
                const char ch = (char)_getch(); 

                /* Pass the character to the Promptly line editor. If the user presses 
                   Enter, the function will return PROMPTLY_END_LINE, indicating that 
                   the line input is complete and can be processed.
                */
                if( promptly_edit_line(&ctx, ch) == PROMPTLY_END_LINE) 
                {
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

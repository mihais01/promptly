#include "promptly.h"
#include <stdio.h>
#include <conio.h>
#include <time.h>   

#ifdef _WIN32
    #include "windows_port.h"
#endif


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
    fflush(stdout);
}

/*
    Print periodic message to demonstrate that the line editing is non-blocking.
*/
static void print_periodic_message(PROMPTLY_CTX)
{
    static clock_t last_print_time = 0;
    static const int interval = 250;
    if(last_print_time == 0) {
        last_print_time = clock();
    }

    clock_t current_time = clock();
    double elapsed_ms = (double)(current_time - last_print_time) * 1000 / CLOCKS_PER_SEC;

    if (elapsed_ms >= interval) 
    {
        promptly_hide(ctx); /* Hide the current line before printing the message */
        printf("[SYSTEM]: Wibbly-wobbly, timey-wimey... stuff.\n"); 
        promptly_show(ctx); /* Re-show the line after printing the message */
        last_print_time = current_time;
    }
}

int main(void)
{
#ifdef _WIN32
    /* Enable ANSI escape code processing on Windows */
      if (!_windows_port_startup()) {
        fprintf(stderr, "Warning: Could not initialize Windows console\n");
    }
#endif

    /* Buffer to hold the user input line. The size is set to 32 characters, which 
       includes the null terminator. This buffer will be used by the Promptly 
       context to store the current line being edited by the user. 
    */
    char line[1024];

    struct promptly_ctx ctx = {
        .prompt = " >>> ",
        .prompt_length = 5,
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
                    printf("You entered: %s\n", ctx.line);
                    break;
                }
            }

            print_periodic_message(&ctx);
        }
    }

    return 0;
}

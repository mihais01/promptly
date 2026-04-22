#pragma once

#include <windows.h>

/* Define this if not available on older SDK versions */
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif


int _windows_port_startup(void) ;

/*
    This function enables virtual terminal processing on Windows console.
    Should be called from main() at startup
*/
int _windows_port_startup(void) 
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    
    if (hOut == INVALID_HANDLE_VALUE || hIn == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error: Cannot get console handle\n");
        return 0;
    }

    /* Enable on STDOUT */
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) {
        fprintf(stderr, "Error: GetConsoleMode failed for STDOUT (error %lu)\n", GetLastError());
        return 0;
    }

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, dwMode)) {
        fprintf(stderr, "Error: SetConsoleMode failed for STDOUT (error %lu)\n", GetLastError());
        return 0;
    }

    /* Also enable on STDIN for input processing */
    dwMode = 0;
    if (!GetConsoleMode(hIn, &dwMode)) {
        fprintf(stderr, "Warning: GetConsoleMode failed for STDIN\n");
    } else {
        dwMode |= ENABLE_VIRTUAL_TERMINAL_INPUT;
        if (!SetConsoleMode(hIn, dwMode)) {
            fprintf(stderr, "Warning: SetConsoleMode failed for STDIN\n");
        }
    }

    return 1;
}
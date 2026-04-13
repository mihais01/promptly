# Promptly

Promptly is a small C line-editing library for terminal applications. It provides an editable input buffer, cursor movement, prompt rendering, and helper functions for temporarily hiding and restoring the current line when asynchronous output needs to be printed.

It performs no dynamic memory allocations and works entirely with buffers owned by the caller.

The project currently focuses on a minimal, embeddable API with a straightforward state machine.

## Current Capabilities

- Editable input line with prompt rendering
- No dynamic allocations
- Character insertion at the cursor position
- Backspace support
- Left and right arrow cursor movement
- Enter detection and completed-line return value
- Bell feedback for invalid operations
- Hide and restore helpers for non-blocking terminal output
- Terminal size probing through DSR escape sequence parsing

## Project Layout

```text
.
|-- CMakeLists.txt
|-- CMakePresets.json
|-- examples/
|   `-- example.c
`-- src/
    |-- promptly.c
    `-- promptly.h
```

## Example

The example in `examples/example.c` shows a non-blocking input loop built around `_kbhit()` and `_getch()`. While the user edits the current line, the program periodically prints a background message and then restores the prompt.

Typical flow:

1. Initialize a `promptly_ctx` with a prompt, line buffer, and write callback.
2. Call `promptly_start_line()` to begin a new editable input session.
3. Feed characters into `promptly_edit_line()`.
4. When it returns `PROMPTLY_END_LINE`, process `ctx.line`.

Minimal setup:

```c
#include "promptly.h"

static void promptly_write(const char *message, size_t length) {
    fwrite(message, 1, length, stdout);
    fflush(stdout);
}

char line[1024];

struct promptly_ctx ctx = {
    .prompt = " >>> ",
    .prompt_length = 5,
    .line = line,
    .line_length = sizeof(line),
    .write = promptly_write,
};

for(;;)
{
    promptly_start_line(&ctx);

    while(1)
    {
        if(_kbhit()) /
        {
            
            const char ch = (char)_getch(); 
            if( promptly_edit_line(&ctx, ch) == PROMPTLY_END_LINE) 
            {
                printf("You entered: %s\n", ctx.line);
                break;
            }
        }
    }
}

```

## Roadmap

- Add context flags
- Add CRLF handling option in the context
- Add bypass commands outside the line editor flow
- Add history
- Add hints
- Add auto-completion
- Improve Line editing capabilities

## License

This project is distributed under the terms in `LICENSE`.

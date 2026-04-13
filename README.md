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

## Build

The repository includes a CMake preset for MinGW on Windows.

```powershell
cmake --preset "GCC 15.2.0 x86_64-w64-mingw32 (ucrt64)"
cmake --build --preset "GCC 15.2.0 x86_64-w64-mingw32 (ucrt64)"
```

If you prefer configuring manually:

```powershell
cmake -S . -B out/build -G Ninja
cmake --build out/build
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

promptly_start_line(&ctx);
```

## Public API

Main declarations are available in `src/promptly.h`.

- `promptly_start_line(ctx)` resets the buffer and starts a new input session
- `promptly_edit_line(ctx, ch)` advances the editor state machine with one input character
- `promptly_show_line(ctx)` redraws the prompt and current buffer
- `promptly_hide(ctx)` clears the visible line before external output
- `promptly_show(ctx)` restores the prompt and input line after external output
- `promptly_bell(ctx)` emits an audible terminal bell

Result values:

- `PROMPTLY_IDLE` means editing is still in progress
- `PROMPTLY_END_LINE` means Enter was pressed and the line is complete
- `PROMPTLY_ERROR` signals invalid state or invalid context

## Notes And Limitations

- The bundled example is Windows-oriented because it uses `conio.h`.
- Extended key handling currently targets the `_getch()` style arrow-key codes used by the example.
- The library is intentionally small and does not yet implement history, hints, or auto-completion.
- Prompt length is provided explicitly through `prompt_length` in the context.
- The caller owns the line buffer and is responsible for choosing its capacity.

## Roadmap

- Add context flags
- Add CRLF handling option in the context
- Add bypass commands outside the line editor flow
- Replace `sscanf` parsing for terminal row and column handling
- Add history
- Add hints
- Add auto-completion

## License

This project is distributed under the terms in `LICENSE`.
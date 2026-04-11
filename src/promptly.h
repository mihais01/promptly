#ifndef PROMPTLY_H
#define PROMPTLY_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define PROMTLY_CTX struct promtly_ctx *const ctx
typedef uint32_t promptly_interface_t;

struct promptly_config {

    /* ===== I/O Operations ===== */

    /*
        * A function pointer to write a message. 
        * The message is not guaranteed to be null-terminated
        *   so the length parameter must be used.
        * The interface parameter can be used to specify which interface to write to, 
        *   for example, stdout or stderr.
    */
    void (*write)(promptly_interface_t interface, const char *message, size_t length);

    /*
        * A function pointer to read input. 
        * The buffer is not guaranteed to be null-terminated
        *   so the length parameter must be used.
        * The interface parameter can be used to specify which interface to read from, 
        *   for example, stdin.
        * The function should return the number of bytes read, 
        *   or a negative value on error.
    */
    ssize_t (*read)(promptly_interface_t interface, char *buffer, size_t length);
};

struct promtly_ctx {
    /* ==== Public configuration ==== */
    promptly_interface_t interface;

    const char *prompt;
    size_t prompt_length;
    
    /* ==== Private Metadata ==== */
    size_t cols;
    size_t rows; 

};

/* @brief Initializes the Promptly library with the provided configuration.
 *
 * @param[in] config A pointer to a promptly_config structure
*/
void promptly_init(const struct promptly_config *const config);

void promtly_start(struct promtly_ctx *const ctx);

void promtly_edit_line(PROMTLY_CTX, 
                            char *const buffer, 
                            size_t *const buffer_size);

void promptly_greet(void);

#endif

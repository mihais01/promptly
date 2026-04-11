#ifndef PROMPTLY_H
#define PROMPTLY_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


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

/* @brief Initializes the Promptly library with the provided configuration.
 *
 * @param[in] config A pointer to a promptly_config structure
*/
void promptly_init(const struct promptly_config *config);

void promptly_greet(void);

#endif

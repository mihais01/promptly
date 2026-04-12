cmake_minimum_required(VERSION 3.10)
project(promptly C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(COMPILE_FLAGS_WARNING
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    -Walloca
    -Wcast-align
    -Wcast-qual
    -Wconversion
    -Wempty-body
    -Wformat=2
    -Wformat-security
    -Wimplicit-function-declaration
    -Wjump-misses-init
    -Wlogical-op
    -Wmissing-declarations
    -Wmissing-field-initializers
    -Wmissing-noreturn
    -Wmissing-prototypes
    -Wnested-externs
    -Wold-style-definition
    -Wpointer-arith
    -Wpointer-to-int-cast
    -Wreturn-type
    -Wshadow
    -Wsign-compare
    -Wsign-conversion
    -Wstrict-aliasing
    -Wstrict-prototypes
    -Wtype-limits
    -Wuninitialized
    -Wundef
    -Wunused
    -Wunused-parameter
    -Wunused-result
    -Wunused-variable
    -Wvla
    -Wwrite-strings
)

set(COMPILE_FLAGS_SECURITY
    -fstack-protector-strong
    -D_FORTIFY_SOURCE=2
    -fPIE
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g3 -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")

add_library(promptly_lib STATIC src/promptly.c)

target_compile_options(promptly_lib PRIVATE 
    ${COMPILE_FLAGS_WARNING}
    ${COMPILE_FLAGS_SECURITY}
)

target_include_directories(promptly_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(UNIX AND NOT APPLE)
    target_compile_options(promptly_lib PRIVATE
        -fsanitize=address
        -fsanitize=undefined
        -fsanitize=leak
    )
    target_link_options(promptly_lib PRIVATE
        -fsanitize=address
        -fsanitize=undefined
        -fsanitize=leak
    )
endif()

add_executable(promptly examples/example.c)

target_link_libraries(promptly PRIVATE promptly_lib)

target_compile_options(promptly PRIVATE 
    ${COMPILE_FLAGS_WARNING}
    ${COMPILE_FLAGS_SECURITY}
)

if(UNIX AND NOT APPLE)
    target_compile_options(promptly PRIVATE
        -fsanitize=address
        -fsanitize=undefined
        -fsanitize=leak
    )
    target_link_options(promptly PRIVATE
        -fsanitize=address
        -fsanitize=undefined
        -fsanitize=leak
    )
endif()

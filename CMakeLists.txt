# set minimum cmake version
cmake_minimum_required(VERSION 3.15)
project(web_server VERSION 1.0 LANGUAGES C)

# set compiler options
set(CMAKE_C_STANDARD 11)        # Use C11 standard
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)     # Enforce standard compliance

# Enable debugging symbols
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS_DEBUG "-g")

# add source files
set(SOURCES
    src/main.c
)

# create the executable
add_executable(web_server ${SOURCES})

# include directories
target_include_directories(web_server PRIVATE src)

# had to add this so networking functions work
add_compile_definitions(_POSIX_C_SOURCE=200112L)

# optional: enable warnings and debugging flags
if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
    target_compile_options(web_server PRIVATE -Wall -Wextra -Wpedantic -g)
endif()

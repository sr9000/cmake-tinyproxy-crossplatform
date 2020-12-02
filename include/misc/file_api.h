//
// Created by sr9000 on 03/12/2020.
//

#ifndef CMAKE_TINYPROXY_FILE_API_H
#define CMAKE_TINYPROXY_FILE_API_H

#include <stdbool.h>

// Safely creates filename and returns the low-level file descriptor.
extern int create_file_safely(const char *filename, bool truncate_file);

// Creates a file with the PID of the Tinyproxy process.
//
// Returns: 0 on success
//          non-zero values on errors
extern int pidfile_create(const char *path);

#endif // CMAKE_TINYPROXY_FILE_API_H

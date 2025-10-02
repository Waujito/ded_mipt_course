#ifndef CT_IO_H
#define CT_IO_H

#include <stdio.h>
#include <stddef.h>
#include <sys/types.h>

ssize_t get_file_size(FILE *file);

int read_file(const char *filename, char **bufptr, size_t *read_bytes_ptr);

#endif /* CT_IO_H */

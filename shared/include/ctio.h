#ifndef CT_IO_H
#define CT_IO_H

#include <stdio.h>
#include <stddef.h>
#include <sys/types.h>
#include "pvector.h"

#ifdef __cplusplus
extern "C" {
#endif

struct text_line {
	char *line_ptr;
	size_t line_sz;
};

ssize_t get_file_size(FILE *file);

int read_file(const char *filename, char **bufptr, size_t *read_bytes_ptr);

/**
 * Reads lines to pvector of type text_line
 */
int read_lines(char *text_buf, size_t buflen, struct pvector *lines_arr);

/**
 * Counts lines in text buffer
 */
size_t count_lines(const char *text_buf, size_t buflen);

/**
 * Initializes pvector with type text_line and reads lines into it
 */
int pvector_read_lines(struct pvector *text_lines, char *buf, size_t buflen);

#ifdef __cplusplus
}
#endif

#endif /* CT_IO_H */

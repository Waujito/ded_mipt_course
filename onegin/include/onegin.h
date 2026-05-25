#ifndef ONEGIN_H
#define ONEGIN_H

#include <stdio.h>
#include <errno.h>
#include <string.h>

#define eprintf(...) fprintf(stderr, __VA_ARGS__)
#define fmtperror(fmt, ...) fprintf(stderr, fmt ": %s\n", ##__VA_ARGS__, strerror(errno))

int process_text_rectangle(const char *in_filename, const char *out_filename);
int process_text_strs_ptrs(const char *in_filename, const char *out_filename);
int process_text_singlebuf(const char *in_filename, const char *out_filename);


static inline int process_text(const char *in_filename, const char *out_filename) {
	return process_text_singlebuf(in_filename, out_filename);
}

#endif /* ONEGIN_H */

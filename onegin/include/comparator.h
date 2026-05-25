#ifndef STR_COMPARATOR_H
#define STR_COMPARATOR_H

#include <stdlib.h>

struct onegin_line {
	char *line_ptr;
	size_t line_sz;
};

int filter_lines(const char *line);

int backward_string_comparator(const void *a1, const void *a2);
int forward_string_comparator(const void *a1, const void *a2);
int strings_comparator(const void *a1, const void *a2);

#endif /* STR_COMPARATOR_H */

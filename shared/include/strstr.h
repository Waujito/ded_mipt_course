#ifndef STRSTR_H
#define STRSTR_H

#include "stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef char *(*strstr_function)(char *haystack, const char *needle);

char *my_strstr_trivial(char *haystack, const char *needle);

char *my_strstr_zfunction(char *haystack, const char *needle);

char *my_strstr_hash(char *haystack, const char *needle);

char *my_strstr_boyer_moore(char *haystack, const char *needle);

#ifdef __cplusplus
}
#endif

#endif /* STRSTR_H */


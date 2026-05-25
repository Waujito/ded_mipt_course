#ifndef COLORS_H
#define COLORS_H

#include <stdio.h>

/**
 * ANSI colors for text in terminal
 */

#define COLOR_RED 	"\033[31m"
#define COLOR_GREEN 	"\033[92m"
#define COLOR_CLEAR 	"\033[0m"

#define FPRINTF_COLORED(color, file, fmt, ...)			\
	fprintf(file, color fmt COLOR_CLEAR, ##__VA_ARGS__)

#endif /* COLORS_H */

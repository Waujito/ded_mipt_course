/**
 * !!!NOTE!!!
 * Stores lines in a array of pointers to strings
 * An educational version, so no new features will be provided for it.
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "types.h"

#include "onegin.h"
#include "comparator.h"
#include "pvector.h"

static void pvector_str_destructor(void *el) {
	char *str = *(char **)el;
	free(str);
}

static int process_text_strs_ptrs_fp(FILE *in_file, FILE *out_file) {
	assert (in_file); 	
	assert (out_file);

	int ret = 0;

	struct pvector lines_arr = {0};
	ret = pvector_init(&lines_arr, sizeof (void *));
	if (ret) {
		log_perror("pvector_init");
		return -1;
	}

	ret = pvector_set_destructor(&lines_arr, pvector_str_destructor);
	if (ret) {
		pvector_destroy(&lines_arr);

		log_perror("pvector_set_destructor");
		return -1;
	}

	char *lineptr = NULL;
	size_t linesz = 0;

	ssize_t read_bytes = 0;
	while ((read_bytes = getline(&lineptr, &linesz, in_file)) >= 0) {
		if (!filter_lines(lineptr)) {
			continue;
		}

		if (pvector_push_back(&lines_arr, &lineptr) < 0) {
			pvector_destroy(&lines_arr);
			return -1;
		}

		// Null up the getline buffer, so it will allocate the new one
		lineptr = NULL;
		linesz = 0;
	}

	free (lineptr);
	lineptr = NULL;

	if (errno) {
		log_perror("Text buffer error");

		pvector_destroy(&lines_arr);
		return -1;
	}

	pvector_sort(&lines_arr, strings_comparator);

	for (size_t i = 0; i < lines_arr.len; i++) {
		fprintf(out_file, "%s", *(const char **)pvector_get(&lines_arr, i));
	}
	printf("text_lines_cnt: <%zu>\n", lines_arr.len);

	pvector_destroy(&lines_arr);

	return 0;
}

int process_text_strs_ptrs(const char *in_filename, const char *out_filename) {
	FILE *in_file = fopen(in_filename, "rb");
	if (!in_file) {
		log_perror("fopen");
		return -1;
	}

	FILE *out_file = fopen(out_filename, "wb");
	if  (!out_file) {
		log_perror("fopen");
		fclose(in_file);
		return -1;
	}

	int ret = process_text_strs_ptrs_fp(in_file, out_file);

	fclose(out_file);
	fclose(in_file);

	return ret;
}

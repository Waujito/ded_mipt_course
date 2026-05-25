/**
 * !!!NOTE!!!
 * A dumbass onegin realization. Stores lines in a 2D array.
 * DO NOT USE THIS
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "vectorized_swap.h"

#include "types.h"

#include "onegin.h"
#include "comparator.h"

struct file_text_parameters {
	size_t max_line_length;
	size_t text_lines_cnt;
};

static int lookup_file_parameters(FILE *text_file, 
				  struct file_text_parameters *params) {
	assert (text_file);
	assert (params);

	/* Find maximum length of all lines */
	size_t max_line_length = 0;
	size_t text_lines_cnt = 0;

	char *lineptr = NULL;
	size_t linesz = 0;

	ssize_t read_bytes = 0;
	while ((read_bytes = getline(&lineptr, &linesz, text_file)) >= 0) {
		if (!filter_lines(lineptr)) {
			continue;
		}

		if (max_line_length < (size_t) read_bytes) {
			max_line_length = (size_t) read_bytes;
		}
		text_lines_cnt++;
	}

	if (errno) {
		log_perror("lookup error");

		free(lineptr);
		return -1;
	}

	free(lineptr);

	printf("Found <%zu> lines with maximum length <%zu>\n", text_lines_cnt, max_line_length);

	/* For \0 at the end of each line */
	max_line_length++;

	/* Move file position indicator to the begginning of the file */
	int ret = fseek(text_file, 0, SEEK_SET);
	if (ret) {
		log_perror("fseek to start");

		free(lineptr);
		return -1;
	}

	params->max_line_length = max_line_length;
	params->text_lines_cnt = text_lines_cnt;

	return 0;
}

static int strings_sort(char *text_array, size_t line_width, size_t lines_cnt) {
	assert (text_array);

	for (size_t i = 0; i < lines_cnt; i++) {
		char *a1 = text_array + i * line_width;
		for (size_t j = 0; j < i; j++) {
			char *a2 = text_array + j * line_width;
			int cmp = strings_comparator(&a1, &a2);

			if (cmp < 0) {
				vectorized_swap(a1, a2, line_width);
			}
		}
	}

	return 0;
}

static int process_text_rectangle_ptrs(FILE *in_file, FILE *out_file) {
	assert (in_file); 	
	assert (out_file);

	struct file_text_parameters file_params = {0};

	int ret = lookup_file_parameters(in_file, &file_params);
	if (ret) {
		return -1;
	}

	size_t max_line_length	= file_params.max_line_length;
	size_t text_lines_cnt	= file_params.text_lines_cnt;

	size_t text_buffer_len = max_line_length * text_lines_cnt;
	char *text_buffer = (char *) calloc(text_buffer_len, sizeof(char));

	if (!text_buffer) {
		log_perror("text buffer allocation error");

		return -1;
	}

	char *text_buffer_ptr	= text_buffer;
	size_t left_text_buffer	= text_buffer_len;

	size_t read_lines = 0;
	while (	left_text_buffer >= max_line_length &&
		fgets(text_buffer_ptr, (int) max_line_length, in_file)) {

		if (!filter_lines(text_buffer_ptr)) {
			continue;
		}

		read_lines++;
		text_buffer_ptr += max_line_length;
		left_text_buffer -= max_line_length;
	}

	if (errno) {
		log_perror("Text buffer error");

		free(text_buffer);
		return -1;
	}

	strings_sort(text_buffer, max_line_length, read_lines);

	text_buffer_ptr = text_buffer;
	for (size_t i = 0; i < read_lines; i++) {
		fprintf(out_file, "%s", text_buffer_ptr);
		text_buffer_ptr += max_line_length;
	}
	printf("text_lines_cnt: <%zu> read_lines: <%zu>\n", text_lines_cnt, read_lines);

	free(text_buffer);

	return 0;
}

int process_text_rectangle(const char *in_filename, const char *out_filename) {
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

	int ret = process_text_rectangle_ptrs(in_file, out_file);

	fclose(out_file);
	fclose(in_file);

	return ret;
}

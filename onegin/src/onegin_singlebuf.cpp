/**
 * USE THIS ONEGIN REALIZATION!
 *
 * Stores the whole text in one array and splits it on lines vector
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>

#include "types.h"
#include "onegin.h"
#include "comparator.h"
#include "pvector.h"

static ssize_t get_file_size(FILE *file) {
#ifndef FGETSIZE_USE_FSEEK
	struct stat file_stat = {0};
	int ret = fstat(fileno(file), &file_stat);
	if (ret) {
		log_perror("get_file_size fstat");
		return -1;
	}

	return file_stat.st_size;
#else /* FGETSIZE_USE_FSEEK */
	ret = fseek(in_file, 0, SEEK_END);
	if (ret) {
		log_perror("fseek to end");
		return -1;
	}

	long fsize = ftell(in_file);
	if (fsize < 0) {
		log_perror("ftell");
		return -1;
	}

	ret = fseek(in_file, 0, SEEK_SET);
	if (ret) {
		log_perror("fseek to start");
		return -1;
	}

	return fsize;
#endif /* FGETSIZE_USE_FSEEK */
}

static int read_file(const char *filename, char **bufptr, size_t *read_bytes_ptr) {
	assert (filename);
	assert (bufptr);
	assert (read_bytes_ptr);

	int ret = S_OK;

	FILE *file = NULL;
	char *text_buf = NULL;
	ssize_t fsize = 0;
	size_t read_bytes = 0;

	if (!(file = fopen(filename, "rb"))) {
		log_perror("fopen(%s, \"rb\")", filename);
		_CT_FAIL();
	}

	if ((fsize = get_file_size(file)) < 0) {
		_CT_FAIL();
	}

	text_buf = (char *)calloc((size_t) fsize + 1, sizeof(char));
	if (!text_buf) {
		log_perror("text arr alloc");
		_CT_FAIL();
	}
	
	read_bytes = fread(text_buf, sizeof(char), (size_t) fsize, file);
	if (ferror(file)) {
		log_perror("fread ferror");
		_CT_FAIL();
	}
	text_buf[read_bytes] = '\0';
	read_bytes++;

	*bufptr = text_buf;

	*read_bytes_ptr = read_bytes;

_CT_EXIT_POINT:
	ct_fclose(file);
	if (_CT_FAILED(ret)) {
		free(text_buf);
	}

	return ret;
}

static int read_lines(char *text_buf, size_t buflen, struct pvector *lines_arr) {
	assert (text_buf);
	assert (lines_arr);

	char *start_lineptr = text_buf;
	size_t line_sz = 1;
	char *cur_lineptr = start_lineptr;

	// left_size is read_bytes instead of read_bytes - 1 because of \0 term
	for (size_t i = 0; i < buflen; i++, cur_lineptr++, line_sz++) {
		if (*cur_lineptr == '\n' || *cur_lineptr == '\0') {
			*cur_lineptr = '\0';

			if (!filter_lines(start_lineptr)) {
				start_lineptr = cur_lineptr + 1;
				line_sz = 0;

				continue;
			}

			assert (line_sz >= 1);

			struct onegin_line o_line = {
				.line_ptr = start_lineptr,
				.line_sz = line_sz - 1,
			};

			if (pvector_push_back(lines_arr, &o_line) < 0) {
				return S_FAIL;
			}

			start_lineptr = cur_lineptr + 1;
			line_sz = 0;
		}
	}

	return S_OK;
}

static size_t count_lines(const char *text_buf, size_t buflen) {
	assert (text_buf);

	size_t nlines = 1;
	for (size_t i = 0; i < buflen; i++) {
		if (text_buf[i] == '\n' || text_buf[i] == '\0')
			nlines++;
	}

	return nlines;
}

static int pvector_read_lines(struct pvector *text_lines, char *buf, size_t buflen) {
	assert (text_lines);
	assert (buf);

	int ret = 0;

	ret = pvector_init(text_lines, sizeof(struct onegin_line));
	if (ret) {
		log_perror("pvector_init");
		return S_FAIL;
	}

	size_t nlines = count_lines(buf, buflen);

	pvector_set_capacity(text_lines, nlines + 1);

	if (read_lines(buf, buflen, text_lines)) {
		pvector_destroy(text_lines);
		return S_FAIL;
	}

	return S_OK;
}

static int print_section_ong(FILE *out_file, const char *label, const struct pvector *onegin_text) {
	assert (out_file);
	assert (label);
	assert (onegin_text);

	fprintf(out_file, "-------%s-------\n", label);
	for (size_t i = 0; i < onegin_text->len; i++) {
		struct onegin_line *o_line = (struct onegin_line *)pvector_get(onegin_text, i);
		fprintf(out_file, "%s\n", o_line->line_ptr);
	}

	return S_OK;
}

int process_text_singlebuf(const char *in_filename, const char *out_filename) {
	assert (in_filename);
	assert (out_filename);

	int ret = S_OK;

	char *text_buf = NULL;
	size_t read_bytes = 0;

	if (read_file(in_filename, &text_buf, &read_bytes)) {
		return S_FAIL;
	}

	struct pvector lines_arr = {0};
	struct pvector forward_arr = {0};
	struct pvector backward_arr = {0};

	_CT_CHECKED(pvector_read_lines(&lines_arr, text_buf, read_bytes));

	printf("text_lines_cnt: <%zu>\n", lines_arr.len);
	
	_CT_CHECKED(pvector_clone(&forward_arr, &lines_arr));
	_CT_CHECKED(pvector_clone(&backward_arr, &lines_arr));

	_CT_CHECKED(pvector_sort(&forward_arr, forward_string_comparator));
	_CT_CHECKED(pvector_sort(&backward_arr, backward_string_comparator));

	{
		FILE *out_file = fopen(out_filename, "wb");
		if (!out_file) {
			ret = S_FAIL;
			_CT_FAIL();
		}

		print_section_ong(out_file, "FORWARD TEXT",	&forward_arr);
		print_section_ong(out_file, "BACKWARD TEXT",	&backward_arr);
		print_section_ong(out_file, "REAL TEXT",	&lines_arr);

		fclose(out_file);
	}

_CT_EXIT_POINT:
	pvector_destroy(&backward_arr);
	pvector_destroy(&forward_arr);
	pvector_destroy(&lines_arr);
	free(text_buf);

	return ret;
}

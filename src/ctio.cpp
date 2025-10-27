#define _GNU_SOURCE
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>

#include "types.h"
#include "ctio.h"
#include "pvector.h"

ssize_t get_file_size(FILE *file) {
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

int read_file(const char *filename, char **bufptr, size_t *read_bytes_ptr) {
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

int read_lines(char *text_buf, size_t buflen, struct pvector *lines_arr) {
	assert (text_buf);
	assert (lines_arr);

	char *start_lineptr = text_buf;
	size_t line_sz = 1;
	char *cur_lineptr = start_lineptr;

	// left_size is read_bytes instead of read_bytes - 1 because of \0 term
	for (size_t i = 0; i < buflen; i++, cur_lineptr++, line_sz++) {
		if (*cur_lineptr == '\n' || *cur_lineptr == '\0') {
			*cur_lineptr = '\0';

			assert (line_sz >= 1);

			struct text_line t_line = {
				.line_ptr = start_lineptr,
				.line_sz = line_sz - 1,
			};

			if (pvector_push_back(lines_arr, &t_line)) {
				return S_FAIL;
			}

			start_lineptr = cur_lineptr + 1;
			line_sz = 0;
		}
	}

	return S_OK;
}

size_t count_lines(const char *text_buf, size_t buflen) {
	assert (text_buf);

	size_t nlines = 1;
	for (size_t i = 0; i < buflen; i++) {
		if (text_buf[i] == '\n' || text_buf[i] == '\0')
			nlines++;
	}

	return nlines;
}

int pvector_read_lines(struct pvector *text_lines, char *buf, size_t buflen) {
	assert (text_lines);
	assert (buf);

	if (pvector_init(text_lines, sizeof(struct text_line))) {
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

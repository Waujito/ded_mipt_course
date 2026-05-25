#include <assert.h>
#include <stdio.h>
#include <stdint.h> 

#include "comparator.h"

int filter_lines(const char *line) {
	assert (line);

	if (*line == '\n')
		return 0;

	if (*line == '\0')
		return 0;

	return 1;
}

static inline int ascii_compare_chars(char c1, char c2) {
	// a-z to A-Z
	if (c1 >= 97 && c1 <= 122) {
		c1 -= 32;
	}
	// a-z to A-Z
	if (c2 >= 97 && c2 <= 122) {
		c2 -= 32;
	}

	if (c1 == c2) {
		return 0;
	} else if (c1 > c2) {
		return 1;
	} else {
		return -1;
	}
}


const char UTF8_STREAM_BYTE_MASK	= 0xc0;
const char UTF8_STREAM_BYTE_VALUE	= 0x80;
const char UTF8_STREAM_SHIFT		= 6;

const char UTF8_ONE_BYTE_MASK		= 0x80;
const char UTF8_ONE_BYTE_VALUE		= 0x00;

const char UTF8_TWO_BYTE_MASK		= 0xe0;
const char UTF8_TWO_BYTE_VALUE		= 0xc0;

const char UTF8_THREE_BYTE_MASK		= 0xf0;
const char UTF8_THREE_BYTE_VALUE	= 0xe0;

const char UTF8_FOUR_BYTE_MASK		= 0xf8;
const char UTF8_FOUR_BYTE_VALUE		= 0xf0;

static inline uint32_t forward_read_utf8_char(const char **bptr, size_t *bptr_size) {
	assert (bptr);
	assert (bptr_size);

	uint32_t cur_sym = 0;
	size_t rb = 0;

	if (*bptr_size == 0) {
		return 0;
	}

	do {
		const char cur_ch = **bptr;	
		(*bptr)++, (*bptr_size)--;

		if ((cur_ch & UTF8_STREAM_BYTE_MASK) == UTF8_STREAM_BYTE_VALUE) {
			cur_sym <<= UTF8_STREAM_SHIFT;
			cur_sym += cur_ch & (~UTF8_STREAM_BYTE_MASK);

			if (rb == 0) {
				return 0;
			}

			rb--;
		} else if ((cur_ch & UTF8_ONE_BYTE_MASK) == UTF8_ONE_BYTE_VALUE) {
			cur_sym = cur_ch & (~UTF8_ONE_BYTE_MASK);
			rb = 0;
		} else if ((cur_ch & UTF8_TWO_BYTE_MASK) == UTF8_TWO_BYTE_VALUE) {
			cur_sym = cur_ch & (~UTF8_TWO_BYTE_MASK);
			rb = 1;
		} else if ((cur_ch & UTF8_THREE_BYTE_MASK) == UTF8_THREE_BYTE_VALUE) {
			cur_sym = cur_ch & (~UTF8_THREE_BYTE_MASK);
			rb = 2;
		} else if ((cur_ch & UTF8_FOUR_BYTE_MASK) == UTF8_FOUR_BYTE_VALUE) {
			cur_sym = cur_ch & (~UTF8_FOUR_BYTE_MASK);
			rb = 3;
		}
	} while (rb && *bptr_size);

	if (rb)
		return 0;

	return cur_sym;
}

static inline uint32_t backward_read_utf8_char(const char **bptr, size_t *bptr_size) {
	assert (bptr);
	assert (bptr_size);

	uint32_t cur_sym = 0;
	size_t rb = 0;

	if (*bptr_size == 0) {
		return 0;
	}

	do {
		const char cur_ch = **bptr;	
		(*bptr)--, (*bptr_size)--;

		uint32_t cc = (unsigned char) cur_ch;

		if ((cur_ch & UTF8_STREAM_BYTE_MASK) == UTF8_STREAM_BYTE_VALUE) {
			cc &=	(~UTF8_STREAM_BYTE_MASK);
			cc <<=	(UTF8_STREAM_SHIFT * rb);
			cur_sym += cc;
			rb++;

			continue;
		}

		if ((cur_ch & UTF8_ONE_BYTE_MASK) == UTF8_ONE_BYTE_VALUE) {
			cur_sym = cur_ch & (~UTF8_ONE_BYTE_MASK);
			break;
		}

		if ((cur_ch & UTF8_TWO_BYTE_MASK) == UTF8_TWO_BYTE_VALUE) {
			cc &= (~UTF8_TWO_BYTE_MASK);
			rb = 1;
		} else if ((cur_ch & UTF8_THREE_BYTE_MASK) == UTF8_THREE_BYTE_VALUE) {
			cc &= (~UTF8_THREE_BYTE_MASK);
			rb = 2;
		} else if ((cur_ch & UTF8_FOUR_BYTE_MASK) == UTF8_FOUR_BYTE_VALUE) {
			cc &= (~UTF8_FOUR_BYTE_MASK);
			rb = 3;
		}
		cc <<= (UTF8_STREAM_SHIFT * rb);
		cur_sym += cc;
		break;

	} while (*bptr_size);

	return cur_sym;
}


static inline int is_utf8_letter(uint32_t c) {
	if (c == 0) {
		return 1;
	}

	// 0-9
	if (c >= 48 && c <= 57) {
		return 1;
	}
	// A-Z
	if (c >= 64 && c <= 90) {
		return 1;
	}
	// a-z
	if (c >= 97 && c <= 122) {
		return 1;
	}
	// А-Яа-яЁё
	if ((c >= 1040 && c <= 1103) || c == 1025 || c == 1105) {
		return 1;
	}

	return 0;
}

static inline int is_ascii_letter(char c) {
	return is_utf8_letter((uint32_t) c);
}

static inline char ascii_to_lower(char c) {
	if (c >= 97 && c <= 122) {
		c -= 32;
	}

	return c;
}

static inline uint32_t utf8_to_lower(uint32_t c) {
	if (c >= 97 && c <= 122) {
		c -= 32;
	}

	// А-Я->а-я
	if (c >= 1040 && c <= 1071) {
		c += 32;
	}

	// Ёё->e
	if (c == 1025 || c == 1105) {
		c = 1077;
	}

	return c;
}

static inline void skip_no_letter(const char **letter) {
	assert (letter);

	while (**letter && !is_ascii_letter(**letter)) {
		(*letter)++;
	}
}

static inline void skip_no_letter_backwards(const char **letter, ssize_t *left_len) {
	assert (letter);
	assert (left_len);

	while (*left_len > 0 && !is_ascii_letter(**letter)) {
		(*letter)--, (*left_len)--;
	}
}

static inline char forward_read_ascii_char(const char **bptr, size_t *bptr_size) {
	assert (bptr);
	assert (bptr_size);

	if (*bptr_size == 0) {
		return 0;
	}

	char c = **bptr;
	(*bptr)++;
	(*bptr_size)--;

	return c;
}

int backward_string_comparator(const void *a1, const void *a2) {
	assert (a1);
	assert (a2);

	const struct onegin_line *ln1 = (const struct onegin_line *)a1;
	const struct onegin_line *ln2 = (const struct onegin_line *)a2;

	size_t left_sz1 = ln1->line_sz;
	size_t left_sz2 = ln2->line_sz;

	if (left_sz1 <= 0 || left_sz2 <= 0) {
		return (int)(left_sz2 - left_sz1);
	}

	const char *t1 = ln1->line_ptr + left_sz1 - 1;
	const char *t2 = ln2->line_ptr + left_sz2 - 1;

	uint32_t c1 = 0;
	uint32_t c2 = 0;

	do {
		do {
			c1 = backward_read_utf8_char(&t1, &left_sz1);
		} while (!is_utf8_letter(c1));

		do {
			c2 = backward_read_utf8_char(&t2, &left_sz2);
		} while (!is_utf8_letter(c2));

		c1 = utf8_to_lower(c1);
		c2 = utf8_to_lower(c2);

		if (c1 < c2) {
			return -1;
		} else if (c1 > c2) {
			return 1;
		}
	} while (left_sz1 && left_sz2);

	if (left_sz1 < left_sz2) {
		return 1;
	} else if (left_sz1 > left_sz2) {
		return -1;
	}

	return 0;
}

int forward_string_comparator(const void *a1, const void *a2) {
	assert (a1);
	assert (a2);

	const struct onegin_line *ln1 = (const struct onegin_line *)a1;
	const struct onegin_line *ln2 = (const struct onegin_line *)a2;

	const char *t1 = ln1->line_ptr;
	const char *t2 = ln2->line_ptr;
	size_t left_t1 = ln1->line_sz;
	size_t left_t2 = ln2->line_sz;

	uint32_t c1 = 0;
	uint32_t c2 = 0;

	do {
		do {
			c1 = forward_read_utf8_char(&t1, &left_t1);
		} while (!is_utf8_letter(c1));

		do {
			c2 = forward_read_utf8_char(&t2, &left_t2);
		} while (!is_utf8_letter(c2));

		c1 = utf8_to_lower(c1);
		c2 = utf8_to_lower(c2);

		if (c1 < c2) {
			return -1;
		} else if (c1 > c2) {
			return 1;
		}
	} while (*t1 != '\0' && *t2 != '\0' && left_t1 && left_t2);

	if (left_t1 < left_t2) {
		return -1;
	} else {
		return 1;
	}
}

int strings_comparator(const void *a1, const void *a2) {
	assert (a1);
	assert (a2);

	const char *t1 = *(const char *const *)a1;
	const char *t2 = *(const char *const *)a2;
	int ret = 0;

	do {
		skip_no_letter(&t1);
		skip_no_letter(&t2);

		if ((ret = ascii_compare_chars(*t1, *t2)) != 0) {
			return ret;
		}
	} while (*(t1++) != '\0' && *(t2++) != '\0');

	return 0;
}

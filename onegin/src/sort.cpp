#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "types.h"

#include "vectorized_swap.h"
#include "sort.h"

int alg_bubble_sort(void *arr, size_t nmemb, size_t size,
		    alg_sorting_comparator comparator) {
	assert (arr);
	assert (comparator);

	for (size_t i = 0; i < nmemb; i++) {
		for (size_t j = 0; j < i; j++) {
			char *i_el = (char *)arr + i * size;
			char *j_el = (char *)arr + j * size;
			int cmp = comparator(i_el, j_el);

			if (cmp < 0) {
				vectorized_swap(i_el, j_el, size);
			}
		}
	}

	return 0;
}

/**
 * Merges two parts of merge sort: [left, mid) and [mid, right)
 * Requires additional memory. Pass this as mergedVectors pointer memory with size of (arr len) * size where size is size of each arr element
 */
static inline int merge_arr(
	void *arr, size_t size, 
	size_t left, size_t mid, size_t right, 
	alg_sorting_comparator comparator,
	void *mergedVectors
) {
	assert (arr);
	assert (comparator);
	assert (mergedVectors);

	size_t left_idx = left, right_idx = mid;
	size_t merged_idx = 0;
	while (left_idx < mid || right_idx < right) {
		size_t cpy_idx = 0;

		if (left_idx >= mid) {
			cpy_idx = right_idx++;
		} else if (right_idx >= right) {
			cpy_idx = left_idx++;
		} else {
			int cmp = comparator((char *)arr + left_idx * size, 
					     (char *)arr + right_idx * size);

			if (cmp >= 0) {
				cpy_idx = right_idx++;
			} else {
				cpy_idx = left_idx++;
			}
		}

		memcpy((char *)mergedVectors + merged_idx * size,
			(char *)arr + cpy_idx * size, size);

		merged_idx++;
	}

	memcpy((char *)arr + left * size, mergedVectors,
		(right - left) * size);

	return 0;
}

int alg_merge_sort(void *arr, size_t nmemb, size_t size,
		   alg_sorting_comparator comparator) {
	assert (arr);
	assert (comparator);

	if (nmemb <= 1) {
		return 0;
	}

	void *mergedVectors = calloc(nmemb, size);
	if (!mergedVectors) {
		log_perror("calloc mergedVectors");
		return -1;
	}


	for (size_t i = 1; i < nmemb; i *= 2) {
		for (size_t j = 0; j < nmemb - i; j += 2 * i) {
			size_t left	= j;
			size_t mid	= j + i;
			size_t right	= j + i + i;

			if (right > nmemb)
				right = nmemb;

			merge_arr(arr, size, left, mid, right,
					comparator, mergedVectors);
		}
	}

	free(mergedVectors);
	
	return 0;
}

int alg_quick_sort(void *arr, size_t nmemb, size_t size,
		   alg_sorting_comparator comparator) {
	assert (arr);
	assert (comparator);

	qsort(arr, nmemb, size, comparator);
	return 0;
}


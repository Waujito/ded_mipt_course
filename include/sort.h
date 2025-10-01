#ifndef SORT_H
#define SORT_H

#include <stdlib.h>

typedef int (*alg_sorting_comparator)(const void *, const void *);

typedef int (*alg_sorting_function)(void *arr, size_t n, size_t size,
					alg_sorting_comparator comparator);


int alg_bubble_sort(void *arr, size_t nmemb, size_t size,
			alg_sorting_comparator comparator);

int alg_merge_sort(void *arr, size_t nmemb, size_t size,
		       alg_sorting_comparator comparator);

int alg_quick_sort(void *arr, size_t nmemb, size_t size,
		       alg_sorting_comparator comparator);


static inline int alg_sort(void *arr, size_t nmemb, size_t size,
			       alg_sorting_comparator comparator) {
	// return alg_bubble_sort(arr, n, size, comparator);
	return alg_merge_sort(arr, nmemb, size, comparator);
	// return alg_quick_sort(arr, nmemb, size, comparator);
}

#endif /* SORT_H */

#ifndef PVECTOR_H
#define PVECTOR_H

#include <stdlib.h>
#include "sort.h"

typedef void (*pvector_el_destructor_t)(void *);
struct pvector {
	char *arr;
	size_t capacity;
	size_t len;
	size_t el_size;

	pvector_el_destructor_t destructor;
};

int pvector_init(struct pvector *pv, size_t el_size);
int pvector_set_destructor(struct pvector *pv, pvector_el_destructor_t destructor);

int pvector_set_capacity(struct pvector *pv, size_t new_cap);

int pvector_destroy(struct pvector *pv);

ssize_t pvector_push_back(struct pvector *pv, void *ptr);
int pvector_pop_back(struct pvector *pv);

int pvector_clone(struct pvector *npv, const struct pvector *pv);

int pvector_has(const struct pvector *pv, size_t idx);
void *pvector_get(const struct pvector *pv, size_t idx);


typedef int (*pv_sorting_function)(struct pvector *lines_arr,
				   alg_sorting_comparator comparator);


int pvector_bubble_sort(struct pvector *lines_arr,
			alg_sorting_comparator comparator);

int pvector_merge_sort(struct pvector *lines_arr,
		       alg_sorting_comparator comparator);

int pvector_quick_sort(struct pvector *lines_arr,
		       alg_sorting_comparator comparator);

static inline int pvector_sort(struct pvector *lines_arr,
			       alg_sorting_comparator comparator) {
	// return pvector_bubble_sort(lines_arr, comparator);
	return pvector_merge_sort(lines_arr, comparator);
}


#endif /* PVECTOR_H */

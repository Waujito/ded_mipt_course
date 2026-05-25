#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pvector.h"
#include "sort.h"

int pvector_init(struct pvector *pv, size_t el_size) {
	assert (pv);
	assert (el_size);

	pv->arr = NULL;
	pv->el_size = el_size;
	pv->capacity = 0;
	pv->len = 0;

	pv->destructor = NULL;

	return 0;
}

int pvector_set_destructor(struct pvector *pv, pvector_el_destructor_t destructor) {
	assert (pv);

	pv->destructor = destructor;

	return 0;
}

int pvector_set_capacity(struct pvector *pv, size_t new_cap) {
	assert (pv);

	if (new_cap < pv->len) {
		return -1;
	}

	if (new_cap == 0) {
		static const size_t PVECTOR_INIT_CAPACITY = 128;
		new_cap = PVECTOR_INIT_CAPACITY;
	}

	char *new_arr = (char *)realloc(pv->arr, 
			    new_cap * pv->el_size);
	if (!new_arr) {
		return -1;
	}

	pv->arr = new_arr;
	pv->capacity = new_cap;
	
	return 0;
}

int pvector_destroy(struct pvector *pv) {
	if (!pv) {
		return 0;
	}

	if (pv->destructor) {
		for (size_t i = 0; i < pv->len; i++) {
			pv->destructor(pv->arr + i * pv->el_size);
		}
	}

	free(pv->arr);
	pv->arr = NULL;
	pv->capacity = 0;
	pv->len = 0;

	return 0;
}

int pvector_clone(struct pvector *npv, const struct pvector *pv) {
	assert (pv);
	assert (npv);

	char *arr = (char *)calloc(pv->len * pv->el_size, sizeof(char));
	if (!arr) {
		return -1;
	}

	npv->arr = arr;
	npv->len = pv->len;
	npv->capacity = pv->len;
	npv->el_size = pv->el_size;

	memcpy(npv->arr, pv->arr, pv->len * pv->el_size);

	return 0;
}

ssize_t pvector_push_back(struct pvector *pv, void *ptr) {
	assert (pv);

	if (pv->len >= pv->capacity) {
		size_t new_cap = pv->capacity * 2;
		if (pvector_set_capacity(pv, new_cap)) {
			return -1;
		}
	}

	size_t idx = pv->len++;
	char *el_pos = pv->arr + idx * pv->el_size;
	memcpy(el_pos, ptr, pv->el_size);

	return (ssize_t) idx;
}

int pvector_pop_back(struct pvector *pv) {
	assert (pv);

	if (pv->len == 0) {
		return -1;
	}

	pv->len--;
	if (pv->destructor) {
		char *el_ptr = pv->arr + pv->len * pv->el_size;
		pv->destructor(el_ptr);
	}

	if (pv->len <= pv->capacity / 4) {
		pvector_set_capacity(pv, pv->len);
	}

	return 0;
}

int pvector_has(const struct pvector *pv, size_t idx) {
	assert (pv);

	return idx < pv->len;
}

void *pvector_get(const struct pvector *pv, size_t idx) {
	assert (pv);

	if (idx >= pv->len) {
		return NULL;
	}

	return pv->arr + idx * pv->el_size;
}

int pvector_bubble_sort(struct pvector *lines_arr, alg_sorting_comparator comparator) {
	assert (lines_arr);
	assert (comparator);

	return alg_bubble_sort(lines_arr->arr, lines_arr->len, lines_arr->el_size, comparator);
}

int pvector_merge_sort(struct pvector *lines_arr, alg_sorting_comparator comparator) {
	assert (lines_arr);
	assert (comparator);

	return alg_merge_sort(lines_arr->arr, lines_arr->len, lines_arr->el_size, comparator);
}

int pvector_quick_sort(struct pvector *lines_arr, alg_sorting_comparator comparator) {
	assert (lines_arr);
	assert (comparator);

	return alg_quick_sort(lines_arr->arr, lines_arr->len, lines_arr->el_size, comparator);
}

#include <string.h>

#include "test_config.h" // IWYU pragma: keep

#include "pvector.h"
#include "sort.h"

static int int_comparator(const void *a1, const void *a2) {
	const int *n1 = (const int *)a1;
	const int *n2 = (const int *)a2;

	if (*n1 == *n2) {
		return 0;
	} else if (*n1 > *n2) {
		return 1;
	} else {
		return -1;
	}
}

static int pv_int_comparator(const void *a1, const void *a2) {
	const int *n1 = *(const int *const *)a1;
	const int *n2 = *(const int *const *)a2;

	return int_comparator(n1, n2);
}

static void test_alg_sorting_function(alg_sorting_function sorting_fn, int *a, size_t arr_sz) {
	int *sorted = (int *)calloc(arr_sz, sizeof(int));
	if (!sorted) {
		ASSERT_EQ(0, 1);
	}
	memcpy(sorted, a, arr_sz * sizeof(int));
	qsort(sorted, arr_sz, sizeof(int), int_comparator);

	int ret = sorting_fn(a, arr_sz, sizeof(int), int_comparator);
	ASSERT_EQ(ret, 0);

	for (size_t i = 0; i < arr_sz; i++) {
		ASSERT_EQ(a[i], sorted[i]);
	}

	free(sorted);
}

static void test_pv_sorting_function(pv_sorting_function sorting_fn, int *a, size_t arr_sz) {
	int *sorted = (int *)calloc(arr_sz, sizeof(int));
	if (!sorted) {
		ASSERT_EQ(0, 1);
	}
	memcpy(sorted, a, arr_sz * sizeof(int));
	qsort(sorted, arr_sz, sizeof(int), int_comparator);

	int ret = 0;
	
	pvector arr = {0};
	ret = pvector_init(&arr, sizeof (void *));
	ASSERT_EQ(ret, 0);

	for (size_t i = 0; i < arr_sz; i++) {
		void *el = a + i;
		pvector_push_back(&arr, &el);
	}

	sorting_fn(&arr, pv_int_comparator);

	for (size_t i = 0; i < arr_sz; i++) {
		ASSERT_EQ(**(int **)pvector_get(&arr, i), sorted[i]);
	}

	pvector_destroy(&arr);
	free(sorted);
}


static void test_alg_sorting_function_default(alg_sorting_function sorting_fn) {
	int a[] = {1 , 3 , -1, 2 , 5 , 0 , 9 , -5, 5};
	size_t arr_sz = sizeof(a) / sizeof(a[0]);

	test_alg_sorting_function(sorting_fn, a, arr_sz);
}

static void test_pv_sorting_function_default(pv_sorting_function sorting_fn) {
	int a[] = {1 , 3 , -1, 2 , 5 , 0 , 9 , -5, 5};
	size_t arr_sz = sizeof(a) / sizeof(a[0]);

	test_pv_sorting_function(sorting_fn, a, arr_sz);
}

TEST(TestSort, BubbleSort) {
	test_alg_sorting_function_default(alg_bubble_sort);
}

TEST(TestSort, MergeSort) {
	test_alg_sorting_function_default(alg_merge_sort);
}

TEST(TestSort, QuickSort) {
	test_alg_sorting_function_default(alg_quick_sort);
}

TEST(TestSort, PVBubbleSort) {
	test_pv_sorting_function_default(pvector_bubble_sort);
}

TEST(TestSort, PVMergeSort) {
	test_pv_sorting_function_default(pvector_merge_sort);
}

TEST(TestSort, PVQuickSort) {
	test_pv_sorting_function_default(pvector_quick_sort);
}

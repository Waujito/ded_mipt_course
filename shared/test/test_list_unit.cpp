#include <stdio.h>
#include "test_machine.h"
#include "list.h"

#define ASSERT_DSERROR(pred, target) ASSERT_EQ((int)(DSError_t)(pred), (int)(DSError_t)(target))

TEST(List, ConstructorDestructor) {
	struct list list = {0};
	ASSERT_DSERROR(list_ctor(&list), DS_OK);
	ASSERT_DSERROR(list_dtor(&list), DS_OK);
}

TEST(List, InsertFirstElement) {
	struct list list = {0};
	list_ptr_t nptr = 0;
	
	ASSERT_DSERROR(list_ctor(&list), DS_OK);
	ASSERT_DSERROR(list_insert(&list, 0, 100, &nptr), DS_OK);
	ASSERT_EQ((int)nptr, 1);
	ASSERT_EQ(list.array[1].value, 100);
	
	list_dtor(&list);
}

TEST(List, InsertMultipleElements) {
	struct list list = {0};
	list_ptr_t nptr = 0;
	
	ASSERT_DSERROR(list_ctor(&list), DS_OK);
	
	ASSERT_DSERROR(list_insert(&list, 0, 10, &nptr), DS_OK);
	ASSERT_DSERROR(list_insert(&list, 1, 20, &nptr), DS_OK);
	ASSERT_DSERROR(list_insert(&list, 2, 30, &nptr), DS_OK);
	
	ASSERT_EQ(list.array[1].value, 10);
	ASSERT_EQ(list.array[2].value, 20);
	ASSERT_EQ(list.array[3].value, 30);
	
	list_dtor(&list);
}

TEST(List, DropElements) {
	struct list list = {0};
	list_ptr_t nptr = 0;
	
	ASSERT_DSERROR(list_ctor(&list), DS_OK);
	
	ASSERT_DSERROR(list_insert(&list, 0, 10, &nptr), DS_OK);
	ASSERT_DSERROR(list_insert(&list, 1, 20, &nptr), DS_OK);
	ASSERT_DSERROR(list_insert(&list, 2, 30, &nptr), DS_OK);
	
	ASSERT_DSERROR(list_drop(&list, 2), DS_OK);
	ASSERT_EQ((int)list.array[1].next, 3);
	ASSERT_EQ((int)list.array[3].prev, 1);
	
	list_dtor(&list);
}

TEST(List, ReuseDroppedElements) {
	struct list list = {0};
	list_ptr_t nptr = 0;
	
	ASSERT_DSERROR(list_ctor(&list), DS_OK);
	
	ASSERT_DSERROR(list_insert(&list, 0, 10, &nptr), DS_OK);
	ASSERT_DSERROR(list_insert(&list, 1, 20, &nptr), DS_OK);
	
	ASSERT_DSERROR(list_drop(&list, 1), DS_OK);
	
	ASSERT_DSERROR(list_insert(&list, 0, 30, &nptr), DS_OK);
	ASSERT_EQ((int)nptr, 1);
	ASSERT_EQ(list.array[1].value, 30);
	
	list_dtor(&list);
}

TEST(List, VerifyEmptyList) {
	struct list list = {0};
	ASSERT_DSERROR(list_ctor(&list), DS_OK);
	ASSERT_DSERROR(list_verify(&list), DS_OK);
	list_dtor(&list);
}

TEST(List, SetCapacity) {
	struct list list = {0};
	ASSERT_DSERROR(list_ctor(&list), DS_OK);
	ASSERT_DSERROR(list_set_capacity(&list, 100), DS_OK);
	ASSERT_EQ((int)list.capacity, 100);
	list_dtor(&list);
}

TEST(List, ComplexOperations) {
	struct list list = {0};
	list_ptr_t nptr = 0;
	
	ASSERT_DSERROR(list_ctor(&list), DS_OK);
	
	for (size_t i = 0; i < 10; i++) {
		ASSERT_DSERROR(list_insert(&list, 0, (int)i * 10, &nptr), DS_OK);
	}
	
	for (list_ptr_t i = 1; i <= 10; i += 2) {
		ASSERT_DSERROR(list_drop(&list, i), DS_OK);
	}
	
	for (size_t i = 0; i < 5; i++) {
		ASSERT_DSERROR(list_insert(&list, 0, (int)i * 100, &nptr), DS_OK);
	}
	
	ASSERT_DSERROR(list_verify(&list), DS_OK);
	
	list_dtor(&list);
}

TEST(List, SequentialDropInsert) {
	struct list list = {0};
	list_ptr_t nptr = 0;
	
	ASSERT_DSERROR(list_ctor(&list), DS_OK);
	
	for (list_ptr_t i = 0; i < 5; i++) {
		ASSERT_DSERROR(list_insert(&list, 0, (int)i, &nptr), DS_OK);
	}
	
	for (list_ptr_t i = 1; i <= 5; i++) {
		ASSERT_DSERROR(list_drop(&list, i), DS_OK);
		ASSERT_DSERROR(list_insert(&list, 0, (int)i * 10, &nptr), DS_OK);
	}
	
	ASSERT_DSERROR(list_verify(&list), DS_OK);
	
	list_dtor(&list);
}

TEST(List, DumpFunctions) {
	struct list list = {0};
	list_ptr_t nptr = 0;
	
	FILE *dot_file = fopen("test.dot", "w");
	if (!dot_file) return;
	
	ASSERT_DSERROR(list_ctor(&list), DS_OK);
	ASSERT_DSERROR(list_insert(&list, 0, 42, &nptr), DS_OK);
	
	ASSERT_DSERROR(list_graph_dump_dot(&list, dot_file), DS_OK);
	
	struct list_dump_params params = {
		.out_stream = stdout,
		.drawing_filename = "test.png"
	};
	ASSERT_DSERROR(list_dump(&list, params), DS_OK);
	
	fclose(dot_file);
	list_dtor(&list);
}

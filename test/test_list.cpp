#include <stdio.h>
#include "test_machine.h"
#include "list.h"

#define ASSERT_DSERROR(pred, target) ASSERT_EQ((int)(pred), (int)(target))

TEST(List, ListOperates) {
	struct list list = {0};
	list_ptr_t nptr = 0;

	FILE *dump_file = fopen("uwu.htm", "w");
	if (!dump_file) {
		log_error("Cannot open dump file!");
		ASSERT_EQ(0, 1);
	}

	struct list_dump_params dump_params = {
		.out_stream = dump_file,
	};

	ASSERT_DSERROR(list_ctor(&list), DS_OK);

	ASSERT_DSERROR(list_insert(&list, 0, 123, &nptr), DS_OK);
	ASSERT_EQ((int)(nptr), 1);

	dump_params.drawing_filename = "list_graph.png";
	list_dump(&list, dump_params);
	fflush(dump_file);

	ASSERT_DSERROR(list_insert(&list, 1, 1, &nptr), DS_OK);
	ASSERT_DSERROR(list_insert(&list, 2, 2, &nptr), DS_OK);
	ASSERT_DSERROR(list_insert(&list, 3, 3, &nptr), DS_OK);
	ASSERT_DSERROR(list_insert(&list, 4, 4, &nptr), DS_OK);

	dump_params.drawing_filename = "list_graph1.png";
	list_dump(&list, dump_params);
	fflush(dump_file);

	ASSERT_DSERROR(list_drop(&list, 1), DS_OK);

	ASSERT_EQ((int)(list.array[0].next), 2);
	ASSERT_EQ((int)(list.array[2].prev), 0);

	ASSERT_DSERROR(list_drop(&list, 2), DS_OK);
	ASSERT_DSERROR(list_drop(&list, 4), DS_OK);

	dump_params.drawing_filename = "list_graph2.png";
	list_dump(&list, dump_params);
	fflush(dump_file);

	ASSERT_DSERROR(list_insert(&list, 0, 5, &nptr), DS_OK);
	ASSERT_EQ((int)(nptr), 4);

	dump_params.drawing_filename = "list_graph3.png";
	list_dump(&list, dump_params);
	fflush(dump_file);

	for (size_t i = 0; i < 4; i++) {
		ASSERT_DSERROR(list_insert(&list, 0, 6 + (int)i, &nptr), DS_OK);
	}
	for (size_t i = 0; i < 4; i++) {
		ASSERT_DSERROR(list_insert(&list, 4, 11 + (int)i, &nptr), DS_OK);
	}
	for (list_ptr_t i = 1; i < 8; i+=2) {
		ASSERT_DSERROR(list_drop(&list, i), DS_OK);
	}
	for (size_t i = 0; i < 4; i++) {
		ASSERT_DSERROR(list_insert(&list, 4, 16 + (int)i, &nptr), DS_OK);
	}
	for (list_ptr_t i = 3; i < 8; i+=2) {
		ASSERT_DSERROR(list_drop(&list, i), DS_OK);
	}
	for (list_ptr_t i = 0; i < 8; i++) {
		ASSERT_DSERROR(list_insert(&list, i, 30 + (int)i, &nptr), DS_OK);
	}
	for (list_ptr_t i = 2; i < 16; i+=3) {
		ASSERT_DSERROR(list_drop(&list, i), DS_OK);
	}
	dump_params.drawing_filename = "list_graph4.png";
	list_dump(&list, dump_params);
	fflush(dump_file);

	dump_params.drawing_filename = "list_graph5.png";
	list_linearize(&list);
	list_dump(&list, dump_params);
	fflush(dump_file);

	fclose(dump_file);

	list_dtor(&list);
}

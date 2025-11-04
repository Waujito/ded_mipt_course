#include <stdio.h>
#include <time.h>

#include "test_machine.h"
#include "list.h"
#include "mlist.h"

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


	list_ptr_t prev_ptrs[5] = {0};

	prev_ptrs[0] = list.array[10].prev;
	list.array[10].prev = 9;
	dump_params.drawing_filename = "list_graph5.png";
	list_dump(&list, dump_params);
	fflush(dump_file);

	prev_ptrs[1] = list.array[3].next;
	list.array[3].next = 1999;
	prev_ptrs[2] = list.array[4].next;
	list.array[4].next = 8939;
	prev_ptrs[3] = list.array[6].prev;
	list.array[6].prev = 9199;
	prev_ptrs[4] = list.array[8].next;
	list.array[8].next = 10;
	dump_params.drawing_filename = "list_graph6.png";
	list_dump(&list, dump_params);
	fflush(dump_file);

	list.array[10].prev = prev_ptrs[0];
	list.array[3].next = prev_ptrs[1];
	list.array[4].next = prev_ptrs[2];
	list.array[6].prev = prev_ptrs[3];
	list.array[8].next = prev_ptrs[4];
	dump_params.drawing_filename = "list_graph7.png";
	list_dump(&list, dump_params);
	fflush(dump_file);


	dump_params.drawing_filename = "list_graph8.png";
	list_linearize(&list);
	list_dump(&list, dump_params);
	fflush(dump_file);

	list_dtor(&list);
}

TEST(List, ListProfileInsert) {
	struct list list = {0};
	ASSERT_DSERROR(list_ctor(&list), DS_OK);

	list_ptr_t nptr = 0;
	for (size_t i = 0; i < 1000000; i++) {
		ASSERT_DSERROR(list_insert(&list, nptr, (int)i, &nptr), DS_OK);
	}

	list_dtor(&list);
}

TEST(MList, MListProfileInsert) {
	struct mlist_node head = {0};

	struct mlist_node *next = &head;
	for (size_t i = 0; i < 1000000; i++) {
		struct mlist_node *new_node = mlist_insert(next, (int)i);
		if (new_node == NULL) {
			ASSERT_EQ(0, 1);
		}

		next = new_node;
	}

	next = head.next;
	while (next != NULL) {
		struct mlist_node *cur = next;
		next = cur->next;
		free(cur);
	}
}

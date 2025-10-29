#ifndef LIST_H
#define LIST_H

#include <stddef.h>

#include "data_structure.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int list_dtype;
typedef size_t list_ptr_t;

typedef struct list_node {
	list_dtype value;

	list_ptr_t next;

	union {
		list_ptr_t prev;
		list_ptr_t list_free_head;
	};
} list_node_t;

struct list {
	list_node_t *array;
	size_t capacity;
	size_t used_capacity;
};


DSError_t list_ctor(struct list *list);

DSError_t list_dtor(struct list *list);

DSError_t list_set_capacity(struct list *list, size_t capacity);

DSError_t list_insert(struct list *list, list_ptr_t parent,
		    list_dtype value, list_ptr_t *new_node_ptr);

DSError_t list_drop(struct list *list, list_ptr_t node_ptr);

struct list_dump_params {
	FILE *out_stream;
	const char *drawing_filename;
};

DSError_t list_verify(struct list *list);

DSError_t list_dump(struct list *list, struct list_dump_params parms);

DSError_t list_graph_dump_dot(struct list *list, FILE *dot_file);

#ifdef __cplusplus
}
#endif

#endif /* LIST_H */


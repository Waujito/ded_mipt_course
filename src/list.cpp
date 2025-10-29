#define _GNU_SOURCE
#include <stdio.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "data_structure.h"
#include "list.h"

#define INITIAL_CAPACITY (128)
#define LIST_ROOT_EL (0)

DSError_t list_ctor(struct list *list) {
	assert (list);

	list->array = NULL;
	list->capacity = 0;
	list->used_capacity = 0;

	return DS_OK;
}

DSError_t list_dtor(struct list *list) {
	assert (list);

	free(list->array);
	list->capacity = 0;
	list->used_capacity = 0;

	return DS_OK;
}

DSError_t list_set_capacity(struct list *list, size_t capacity) {
	assert (list);

	if (capacity < list->used_capacity) {
		return DS_INVALID_ARG;
	}

	size_t old_capacity = list->capacity;

	list_node_t *newptr = 
		(list_node_t *)realloc(list->array, capacity * sizeof(list_node_t));

	if (!newptr) {
		return DS_ALLOCATION;
	}

	list->array = newptr;
	list->capacity = capacity;

	// Init 0th element
	if (old_capacity == 0 && capacity > 0) {
		list->used_capacity = 1;

		list_node_t *root_node = list->array;

		root_node->next			= LIST_ROOT_EL;
		root_node->list_free_head	= LIST_ROOT_EL;
	}

	return DS_OK;
}

static inline DSError_t list_increment_capacity(struct list *list) {
	assert (list);

	size_t new_capacity = list->capacity * 2;
	if (new_capacity < INITIAL_CAPACITY) {
		new_capacity = INITIAL_CAPACITY;
	}

	return list_set_capacity(list, new_capacity);
}

static DSError_t find_free_ptr(struct list *list, list_ptr_t *node_ptr) {
	assert (list);
	assert (node_ptr);


	list_node_t *root_node = NULL;
	list_node_t *found_node = NULL;
	DSError_t ret = DS_OK;

	if (!list->array) {
		_CT_CHECKED(list_increment_capacity(list));
	}

	root_node = list->array;

	if (root_node->list_free_head != LIST_ROOT_EL) {
		*node_ptr = root_node->list_free_head;
		found_node = &list->array[*node_ptr];
		root_node->list_free_head = found_node->next;
		list->array[root_node->list_free_head].prev = LIST_ROOT_EL;
	} else {
		if (list->used_capacity >= list->capacity) {
			_CT_CHECKED(list_increment_capacity(list));
		}

		*node_ptr = list->used_capacity++;
		found_node = &list->array[*node_ptr];
	}

	found_node->prev = LIST_ROOT_EL;
	found_node->next = LIST_ROOT_EL;

_CT_EXIT_POINT:
	return ret;
}

DSError_t list_insert(struct list *list, list_ptr_t parent,
		    list_dtype value, list_ptr_t *new_node_ptr) {
	assert (list);
	
	list_ptr_t node_ptr = 0;
	list_node_t *new_node = NULL;
	DSError_t ret = DS_OK;
	list_node_t *parent_node = NULL;

	_CT_CHECKED(find_free_ptr(list, &node_ptr));

	parent_node	= &list->array[parent];
	new_node	= &list->array[node_ptr];

	new_node->prev		= parent;
	new_node->next		= parent_node->next;
	parent_node->next	= node_ptr;

	if (new_node->next != LIST_ROOT_EL) {
		list->array[new_node->next].prev = node_ptr;
	}
	
	if (new_node_ptr) {
		*new_node_ptr = node_ptr;
	}

	new_node->value = value;

_CT_EXIT_POINT:
	return ret;
}

DSError_t list_drop(struct list *list, list_ptr_t node_ptr) {
	assert (list);

	if (node_ptr == LIST_ROOT_EL) {
		return DS_INVALID_ARG;
	}

	list_node_t *node = &list->array[node_ptr];
	list_node_t *root = list->array;

	list->array[node->prev].next = node->next;	

	if (node->next != LIST_ROOT_EL) {
		list->array[node->next].prev = node->prev;
	}

	node->next = root->list_free_head;
	if (root->list_free_head != LIST_ROOT_EL) {
		list->array[root->list_free_head].prev = node_ptr;
	}
	root->list_free_head = node_ptr;
	node->prev = LIST_ROOT_EL;

	return DS_OK;
}

DSError_t list_verify(struct list *list) {
	assert (list);

	DSError_t ret = 0;

	if (list->used_capacity > list->capacity) {
		ret |= DS_STRUCT_CORRUPT;
	}

	list_node_t *root = list->array;
	if (!root) {
		return ret;
	}

	size_t gathered_capacity = 1;
	list_ptr_t rprev = LIST_ROOT_EL;

	for (list_ptr_t inode = root->next; inode != LIST_ROOT_EL;
			inode = list->array[inode].next) {
		list_node_t *node = &list->array[inode];

		if (node->prev != rprev) {
			ret |= DS_LIST_CONN_BACKWARD_CORRUPT;
		}

		rprev = inode;
		gathered_capacity++;
	}

	rprev = LIST_ROOT_EL;
	for (list_ptr_t inode = root->list_free_head; inode != LIST_ROOT_EL;
			inode = list->array[inode].next) {
		list_node_t *node = &list->array[inode];

		if (node->prev != rprev) {
			ret |= DS_LIST_CONN_BACKWARD_CORRUPT;
		}

		rprev = inode;
		gathered_capacity++;
	}

	if (gathered_capacity != list->used_capacity) {
		ret |= DS_LIST_CONNECTIVITY_CORRUPT;
	}


	return ret;
}

static DSError_t dump_draw_dot(struct list *list, const char *drawing_filename);

static DSError_t dump_elements(struct list *list,
			       struct list_dump_params *params) {
#define DUMP_LOG(...) fprintf(params->out_stream, __VA_ARGS__)

	assert (list);
	assert (params);
	assert (params->out_stream);

	list_node_t *root = list->array;
	if (!root) {
		return DS_INVALID_STATE;
	}

	DUMP_LOG("\n\tLinked elements: {\n");
	for (list_ptr_t inode = root->next; inode != LIST_ROOT_EL;
			inode = list->array[inode].next) {
		list_node_t *node = &list->array[inode];
		DUMP_LOG("\t\tnode: %zu prev: %zu next: %zu value: %d\n",
			inode, node->prev, node->next,
			node->value);
	}
	DUMP_LOG("\t}\n\tLinked frees: {\n");

	for (list_ptr_t inode = root->list_free_head; inode != LIST_ROOT_EL;
			inode = list->array[inode].next) {
		list_node_t *node = &list->array[inode];
		DUMP_LOG("\t\tnode: %zu prev: %zu next: %zu value: %d\n",
			inode, node->prev, node->next,
			node->value);
	}
	DUMP_LOG("\t}\n");

#undef DUMP_LOG

	return DS_OK;
}

DSError_t list_dump(struct list *list,
		    struct list_dump_params params) {
#define DUMP_LOG(...) fprintf(params.out_stream, __VA_ARGS__)

	assert (list);

	if (!params.out_stream) {
		params.out_stream = stderr;
	}

	DSError_t ret = DS_OK;

	DUMP_LOG("<pre>\n");

	DUMP_LOG("<h3>List dump:</h3>\n");
	DUMP_LOG("\tLinear len: %zu\n", list->used_capacity);
	DUMP_LOG("\tCapacity: %zu\n", list->capacity);
	DUMP_LOG("\tArray: %p\n", list->array);

	DSError_t verifier_verdict = list_verify(list);
	DUMP_LOG("\n\tVerifier verdict: <0x%x> \n\t\t", verifier_verdict);

	fprint_DSError(params.out_stream, verifier_verdict);
	DUMP_LOG("\n\n");

	list_node_t *root = list->array;
	if (!root) {
		DUMP_LOG("\t[EMPTY LIST]\n");
	} else {
		_CT_CHECKED(dump_elements(list, &params));
	}

	if (params.drawing_filename) {
		_CT_CHECKED(dump_draw_dot(list, params.drawing_filename));
		DUMP_LOG("<img src=\"%s\" />\n", params.drawing_filename);
	}

	
_CT_EXIT_POINT:
	DUMP_LOG("</pre>\n");

	if (ret) {
		DUMP_LOG("<h3>DUMP ERROR!</h3>\n");
	}

#undef DUMP_LOG

	return DS_OK;
}

static DSError_t dump_draw_dot(struct list *list, const char *drawing_filename) {
	#define DOT_PREFIX "dot -Tpng -o %s"

	DSError_t ret = 0;

	size_t strsize = strlen(DOT_PREFIX) + strlen(drawing_filename);
	FILE *dot_file = NULL;
	int sn_written = 0;

	char *ct_string = (char *)calloc(strsize, sizeof(char));

	if (!ct_string) {
		_CT_CHECKED(DS_ALLOCATION);
	}

	sn_written = snprintf(ct_string, strsize, DOT_PREFIX, drawing_filename);

	if (sn_written < 0 || sn_written > (int)strsize) {
		_CT_CHECKED(DS_ALLOCATION);
	}


	dot_file = popen(ct_string, "w");
	if (!dot_file) {
		_CT_CHECKED(DS_ALLOCATION);
	}

	_CT_CHECKED(list_graph_dump_dot(list, dot_file));	

_CT_EXIT_POINT:
	if (dot_file && pclose(dot_file)) {
		ret |= DS_INVALID_STATE;
	}
	free(ct_string);
	return ret;
}

DSError_t list_graph_dump_dot(struct list *list, FILE *dot_file) {
	assert(list);

#define DOT_PRINTF(...) fprintf(dot_file, __VA_ARGS__)
    
	DOT_PRINTF("digraph LinkedList {\n");
	DOT_PRINTF("rankdir=LR;\n");
	DOT_PRINTF("node [shape=Mrecord, style=filled];\n");
	DOT_PRINTF("edge [shape=inv, arrowsize=2.0];\n\n");
	// DOT_PRINTF("splines=ortho;\n");

	list_node_t *root = list->array;
    
	if (!root) {
		DOT_PRINTF("}\n");
		return DS_OK;
	}

	DOT_PRINTF("node0 [label=\"<f0> ROOT | <f1> list_free_head=%zu | <f2> next=%zu | <f3> value=%d\", fillcolor=\"%s\"];\n", root->prev, root->next, root->value, "lightblue");

	for (list_ptr_t inode = root->next; inode != LIST_ROOT_EL;
			inode = list->array[inode].next) {
		list_node_t *node = &list->array[inode];

		DOT_PRINTF("node%zu [label=\"<f0> %zu | <f1> prev=%zu | <f2> next=%zu | <f3> value=%d\", fillcolor=\"%s\"];\n", inode, inode, node->prev, node->next, node->value, "lightgreen");
	}
    
	for (list_ptr_t inode = root->list_free_head; inode != LIST_ROOT_EL;
			inode = list->array[inode].next) {
		list_node_t *node = &list->array[inode];

		DOT_PRINTF("node%zu [label=\"<f0> %zu | <f1> prev=%zu | <f2> next=%zu | <f3> value=%d\", fillcolor=\"%s\"];\n", inode, inode,
	    node->prev, node->next, node->value, "lightcoral");
	}

	// /*
	DOT_PRINTF("{node0 ->");
	for (size_t i = 1; i < list->used_capacity; i++) {
		DOT_PRINTF("node%zu", i);

		if (i != list->used_capacity - 1) {
			DOT_PRINTF("->");
		}
	}
	DOT_PRINTF("[style=invis]}");
	// */
	// DOT_PRINTF("{rank=same;");
	// for (size_t i = 0; i < list->used_capacity; i++) {
	// 	DOT_PRINTF("node%zu;", i);
	// }
	// DOT_PRINTF("};\n");
	// DOT_PRINTF("[splines=ortho,styles=,rank=same];\n");
    
	for (list_ptr_t inode = root->next; inode != LIST_ROOT_EL;
			inode = list->array[inode].next) {
		list_node_t *node = &list->array[inode];


		// DOT_PRINTF("node%zu [label=\"<f0> %zu | <f1> prev=%zu | <f2> next=%zu | <f3> value=%d\", fillcolor=\"%s\"];\n", inode, inode, node->prev, node->next, node->value, "lightgreen");
		if (node->next != LIST_ROOT_EL) {
			DOT_PRINTF("node%zu -> node%zu [constraint=false];\n", inode, node->next);
		}
		DOT_PRINTF("node%zu -> node%zu [constraint=false, color=\"blue\"];\n", inode, node->prev);
	}
    
	for (list_ptr_t inode = root->list_free_head; inode != LIST_ROOT_EL;
			inode = list->array[inode].next) {
		list_node_t *node = &list->array[inode];

		// DOT_PRINTF("node%zu [label=\"<f0> %zu | <f1> prev=%zu | <f2> next=%zu | <f3> value=%d\", fillcolor=\"%s\"];\n", inode, inode,
	    // node->prev, node->next, node->value, "lightcoral");

		if (node->next != LIST_ROOT_EL) {

		DOT_PRINTF("node%zu -> node%zu [constraint=false, color=\"red\"];\n", 
			    inode, node->next);
		}
		// eprintf("gav %zu %zu\n", inode, node->prev);
		DOT_PRINTF("node%zu -> node%zu [constraint=false, color=\"gray\"];\n", 
			    inode, node->prev);
	}	

	DOT_PRINTF("node0 -> node%zu;\n", root->next);

	DOT_PRINTF("\tnode0 -> node%zu [color=\"red\"];\n", 
		root->list_free_head);

    
	DOT_PRINTF("}\n");

#undef DOT_PRINTF
    
	return DS_OK;
}

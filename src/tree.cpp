#define _GNU_SOURCE
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include "hash.h"

#include "tree.h"
#include "data_structure.h"

DSError_t tree_ctor(struct tree *tree) {
	assert (tree);

	tree->root = NULL;
	
	return DS_OK;
}

DSError_t tree_dtor(struct tree *tree) {
	assert (tree);

	tnode_dtor(tree->root);

	return DS_OK;
}

// DSError_t tree_insert(struct tree *tree,
// 		      struct tree_node *node) {
//
// }

struct tree_node *tnode_ctor(void) {
	return (struct tree_node *)
		calloc(1, sizeof(struct tree_node));
}

void tnode_dtor(struct tree_node *node) {
	assert (node);

	if (node->left) {
		tnode_dtor(node->left);
	}
	if (node->right) {
		tnode_dtor(node->right);
	}
	
	free(node);
}

// DSError_t tree_dump(struct tree *tree) {
// }

static DSError_t dump_draw_dot(struct tree *list, const char *drawing_filename);

static DSError_t dump_elements(struct tree *tree,
			       struct tree_dump_params *params) {
#define DUMP_LOG(...) fprintf(params->out_stream, __VA_ARGS__)

	assert (tree);
	assert (params);
	assert (params->out_stream);

	if (!tree->root) {
		return DS_INVALID_STATE;
	}

	/*
	DUMP_LOG("\n\tLinked elements: {\n");
	for (size_t i = 0; i < list->used_capacity; i++) {
		list_node_t *node = &list->array[i];

		if (node->prev != LIST_PREV_FREE) {
			DUMP_LOG("\t\tnode: %zu prev: %zd next: %zd value: %d\n",
			i, node->prev, node->next,
			node->value);

		}
	}

	DUMP_LOG("\t}\n\tLinked frees: {\n");
	for (size_t i = 0; i < list->used_capacity; i++) {
		list_node_t *node = &list->array[i];

		if (node->prev == LIST_PREV_FREE) {
			DUMP_LOG("\t\tnode: %zu prev: %zd next: %zd value: %d\n",
			i, node->prev, node->next,
			node->value);

		}
	}
	DUMP_LOG("\t}\n");
	*/

#undef DUMP_LOG

	return DS_OK;
}

DSError_t tree_dump(struct tree *tree,
		    struct tree_dump_params params) {
#define DUMP_LOG(...) fprintf(params.out_stream, __VA_ARGS__)

	assert (tree);

	if (!params.out_stream) {
		params.out_stream = stderr;
	}

	DSError_t ret = DS_OK;

	DUMP_LOG("<pre>\n");

	DUMP_LOG("<h3>Tree dump:</h3>\n");
	DUMP_LOG("\tRoot ptr: %p\n", tree->root);

	// DSError_t verifier_verdict = tree_verify(tree);
	// DUMP_LOG("\n\tVerifier verdict: <0x%x> \n\t\t", verifier_verdict);
	// fprint_DSError(params.out_stream, verifier_verdict);

	DUMP_LOG("\n\n");

	struct tree_node *root = tree->root;
	if (!root) {
		DUMP_LOG("\t[EMPTY TREE]\n");
	} else {
		_CT_CHECKED(dump_elements(tree, &params));
	}

	if (params.drawing_filename) {
		_CT_CHECKED(dump_draw_dot(tree, params.drawing_filename));
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

static DSError_t dump_draw_dot(struct tree *tree, const char *drawing_filename) {
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

	_CT_CHECKED(tree_graph_dump_dot(tree, dot_file));	

_CT_EXIT_POINT:
	if (dot_file && pclose(dot_file)) {
		ret |= DS_INVALID_STATE;
	}
	free(ct_string);
	return ret;
}

static uint32_t tree_pointer_hash(void *ptr) {
	uint32_t hsh = hash_crc32((uint8_t *)&ptr, sizeof(ptr));
	hsh >>= 8;

	uint32_t ncolor = 0x0;

	ncolor |= ((hsh & 0xFF) / 2 + 0x80);
	ncolor |= (((hsh >> 8) & 0xFF) / 2 + 0x80) << 8;
	ncolor |= (((hsh >> 16) & 0xFF) / 2 + 0x80) << 16;

	return ncolor;
}

static uint32_t inverse_color(uint32_t color) {
	uint32_t ncolor = 0x00;

	return ncolor;
}

static DSError_t tree_dump_node(struct tree_node *node, size_t *node_idx,
				FILE *dot_file) {
	assert (node);
	assert (node_idx);

	DSError_t ret = DS_OK;

#define DOT_PRINTF(...) fprintf(dot_file, __VA_ARGS__)

	size_t cnode_idx = *node_idx;

	const char *box_color = "lightgreen";

	if (cnode_idx == 0) {
		box_color = "lightblue";
	}

	DOT_PRINTF("node%zu [label=<"
		"<TABLE><TR><TD>value=%d</TD></TR>"
		"<TR><TD BGCOLOR=\"#%06x\"><FONT  COLOR=\"#%06x\">self=%p</FONT></TD></TR>"
		"<TR><TD BGCOLOR=\"#%06x\"><FONT  COLOR=\"#%06x\">left=%p</FONT></TD></TR>"
		"<TR><TD BGCOLOR=\"#%06x\"><FONT  COLOR=\"#%06x\">right=%p</FONT></TD></TR>"
		"</TABLE>>, fillcolor=\"%s\", shape=Mrecord];\n", 
		cnode_idx, node->value,
		tree_pointer_hash(node), inverse_color(tree_pointer_hash(node)), node,
		tree_pointer_hash(node->left), inverse_color(tree_pointer_hash(node->left)), node->left,
		tree_pointer_hash(node->right), inverse_color(tree_pointer_hash(node->right)), node->right,
		box_color
	);

	(*node_idx)++;

	if (node->left) {
		size_t lnode_idx = *node_idx;

		DOT_PRINTF("node%zu -> node%zu [color=blue];", cnode_idx, lnode_idx);

		_CT_CHECKED(tree_dump_node(node->left, node_idx, dot_file));
	}

	if (node->right) {
		size_t rnode_idx = *node_idx;

		DOT_PRINTF("node%zu -> node%zu [color=red];", cnode_idx, rnode_idx);

		_CT_CHECKED(tree_dump_node(node->right, node_idx, dot_file));
	}

#undef DOT_PRINTF

_CT_EXIT_POINT:
	return ret;
}

DSError_t tree_graph_dump_dot(struct tree *tree, FILE *dot_file) {
	assert(tree);

#define DOT_PRINTF(...) fprintf(dot_file, __VA_ARGS__)

	DOT_PRINTF("digraph LinkedList {\n");
	DOT_PRINTF("rankdir=LC;\n");
	DOT_PRINTF("node [shape=tripleoctagon, style=filled, fillcolor=red];\n");
	DOT_PRINTF("edge [shape=inv, arrowsize=1.0];\n\n");

	if (tree->root) {
		size_t node_idx = 0;
		tree_dump_node(tree->root, &node_idx, dot_file);
	}

	DOT_PRINTF("}\n");

#undef DOT_PRINTF
    
	return DS_OK;
}

/*
#ifndef TREE_H
#define TREE_H

#include "data_structure.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int tree_dtype;

struct tree_node {
	struct tree_node *left;
	struct tree_node *right;
	
	tree_dtype value;
};

struct tree {
	struct tree_node *root;
};

DSError_t tree_ctor(struct tree *tree);
DSError_t tree_dtor(struct tree *tree);

// DSError_t tree_insert(struct tree *tree,
// 		struct tree_node *node);

struct tree_node *tnode_ctor(void);
void tnode_dtor(struct tree_node *node);

// DSError_t tree_drop(struct tree *tree,
// 	      struct tree_node *node);

struct tree_dump_params {
	FILE *out_stream;
	const char *drawing_filename;
};

DSError_t tree_dump(struct tree *tree,
		    struct tree_dump_params params);

DSError_t tree_graph_dump_dot(struct tree *tree, FILE *dot_file);

#ifdef __cplusplus
}
#endif

#endif /* TREE_H */
*/

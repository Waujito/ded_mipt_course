#include <assert.h>
#include <stdlib.h>

#ifndef MLIST_H
#define MLIST_H

typedef int mlist_dtype;

struct mlist_node {
	struct mlist_node *prev;
	struct mlist_node *next;

	mlist_dtype value;
};

static struct mlist_node *mlist_insert(struct mlist_node *node,
				     mlist_dtype value) {
	assert(node);

	struct mlist_node *next_node = (struct mlist_node *)calloc(
				1, sizeof(struct mlist_node));

	if (!next_node) {
		return NULL;
	}

	next_node->next = node->next;
	next_node->prev = node;
	node->next = next_node;

	next_node->value = value;

	return next_node;
}

static struct mlist_node *mlist_drop(struct mlist_node *node) {
	assert(node);

	if (node->prev) {
		node->prev->next = node->next;
	}

	if (node->next) {
		node->next->prev = node->prev;
	}

	struct mlist_node *node_next = node->next;
	free(node);

	return node_next;
}

#endif /* MLIST_H */

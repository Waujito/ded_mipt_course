#include "test_config.h"
#include "tree.h"

#define ASSERT_DSERROR(pred, target) ASSERT_EQ((int)(pred), (int)(target))

TEST(Tree, TreeDumps) {
	struct tree tree = {0};

	ASSERT_DSERROR(tree_ctor(&tree), DS_OK);

	struct tree_node *node = tnode_ctor();
	ASSERT_EQ((int)(node != NULL), 1);
	tree.root = node;

	node = tnode_ctor();
	ASSERT_EQ((int)(node != NULL), 1);
	tree.root->left = node;

	node = tnode_ctor();
	ASSERT_EQ((int)(node != NULL), 1);
	tree.root->left->left = node;

	node = tnode_ctor();
	ASSERT_EQ((int)(node != NULL), 1);
	tree.root->right = node;

	struct tree_node *tnode = tree.root->left;
	for (size_t i = 0; i < 15; i++) {
		node = tnode_ctor();
		ASSERT_EQ((int)(node != NULL), 1);

		if (i % 5 == 0 && !tnode->right) {
			tnode->right = node;
			tnode = node;
		} else if (i % 5 == 1 && !tnode->left) {
			tnode->left = node;
		} else if (i % 5 == 2 && !tnode->left) {
			tnode->left = node;
		} else if (i % 5 == 3 && !tnode->right) {
			tnode->right = node;
			tnode = node;
		} else {
			if (tnode->left) {
				tnode = tnode->left;
				tnode->left = node;
			} else if (tnode->right) {
				tnode = tnode->right;
				tnode->left = node;
			} else {
				tnode_dtor(node);
			}
		}
	}

	tnode = tree.root->right;
	for (size_t i = 0; i < 15; i++) {
		node = tnode_ctor();
		ASSERT_EQ((int)(node != NULL), 1);

		if (i % 5 == 0 && !tnode->right) {
			tnode->right = node;
			tnode = node;
		} else if (i % 5 == 1 && !tnode->left) {
			tnode->left = node;
		} else if (i % 5 == 2 && !tnode->left) {
			tnode->left = node;
		} else if (i % 5 == 3 && !tnode->right) {
			tnode->right = node;
			tnode = node;
		} else {
			if (tnode->left) {
				tnode = tnode->left;
				tnode->left = node;
			} else if (tnode->right) {
				tnode = tnode->right;
				tnode->left = node;
			} else {
				tnode_dtor(node);
			}
		}
	}

	FILE *dump_file = fopen("tree.htm", "w");
	if (!dump_file) {
		log_error("Cannot open dump file!");
		ASSERT_EQ(0, 1);
	}

	struct tree_dump_params dump_params = {
		.out_stream = dump_file,
	};

	dump_params.drawing_filename = "tree_graph.png";
	tree_dump(&tree, dump_params);
	fflush(dump_file);

	tree_dtor(&tree);
	fclose(dump_file);
}


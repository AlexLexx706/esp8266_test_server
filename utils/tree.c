#include "tree.h"
#include "assert.h"
#include <stddef.h>

#ifdef DEBUG_TREE
	#include <stdio.h>
#endif

void tree_init(Tree * tree) {
	assert(tree);
	#ifdef DEBUG_TREE
	fprintf(stderr, "%s tree:%p\n", __FUNCTION__, tree);
	#endif
	tree->parent = NULL;
	LIST_INIT(&tree->child_head);
}

int tree_add_child(Tree * tree, Tree * child, int reset_childs) {
	List * pos;
	Tree * item;
	assert(tree);
	assert(child);

	#ifdef DEBUG_TREE
	fprintf(stderr, "%s -> tree:%p child:%p\n", __FUNCTION__, tree, child);
	#endif

	LIST_ITER(&tree->child_head) {
		item = LIST_ITEM(Tree, head, pos);
		#ifdef DEBUG_TREE
		fprintf(stderr, "%s child_item:%p\n", __FUNCTION__, item);
		#endif

		if (item == child)
			return 1;
	}

	list_add_tail(&tree->child_head, &child->head);

	if (reset_childs)
		LIST_INIT(&child->child_head);

	child->parent = tree;

	#ifdef DEBUG_TREE
	fprintf(stderr, "%s <-\n", __FUNCTION__);
	#endif
	return 0;
}

int tree_remove_child(Tree * tree, Tree * child) {
	List * pos;
	Tree * item;
	assert(tree);
	assert(child);
	#ifdef DEBUG_TREE
	fprintf(stderr, "%s -> tree:%p child:%p\n", __FUNCTION__, tree, child);
	#endif

	LIST_ITER(&tree->child_head) {
		item = LIST_ITEM(Tree,	head, pos);
		if (item == child) {
			list_remove(pos);
			#ifdef DEBUG_TREE
			fprintf(stderr, "%s <- ok\n", __FUNCTION__);
			#endif

			return 0;
		}
	}
	#ifdef DEBUG_TREE
	fprintf(stderr, "%s <- fail\n", __FUNCTION__);
	#endif
	return 1;
}

Tree * tree_search(Tree * tree, tree_check_fun fun, int recursive) {
	List * pos;
	Tree * item;
	assert(tree);
	assert(fun);

	LIST_ITER(&tree->child_head) {
		item = LIST_ITEM(Tree,	head, pos);
		if (fun(item))
			return item;
		if (recursive) {
			if ( (item = tree_search(item, fun, recursive)) ) {
				return item;
			}
		}
	}
	return NULL;
}


#ifdef TEST_TREE
#include <stdio.h>
#include <string.h>

typedef struct 
{
	const char * name;
	int value;
	Tree tree;
} GrilTreeItem;


Tree * check_c(Tree* item) {
	GrilTreeItem * data;
	assert(item);
	data = LIST_ITEM(GrilTreeItem, tree, item);

	fprintf(stderr, "%s data->name:%s, data->value:%d\n",
		__FUNCTION__, data->name, data->value);

	if (data->value == 3)
		return item;
	return NULL; 
}

GrilTreeItem * find_gril_tree_item(GrilTreeItem * head_item, const char * path) {
	List * pos;
	GrilTreeItem * item;
	assert(path);
	assert(head_item);
	const char * end = strchr(path, '/');

	#ifdef DEBUG_TREE
	fprintf(stderr, "%s item->name:%s path:%s\n",
		__FUNCTION__, head_item->name, path);
	#endif

	//has childs
	if (end) {
		int name_len = end - path;
		LIST_ITER(&(head_item->tree.child_head)) {
			item = LIST_ITEM(GrilTreeItem, tree.head, pos);

			if (!strncmp(item->name, path, name_len)) {
				return find_gril_tree_item(item, end + 1);
			}
		}
		return NULL;
	//no childs
	} else {
		LIST_ITER(&(head_item->tree.child_head)) {
			item = LIST_ITEM(GrilTreeItem, tree.head, pos);
			if (!strcmp(item->name, path)) {
				return item;
			}
		}
		return NULL;
	}
}

int main() {
	GrilTreeItem tree_items[] = {{"/", 0}, {"a", 1}, {"b", 2}, {"c", 3}, {"d", 4}};
	int i;

	//init tree in items
	fprintf(stderr, "%s\n", __FUNCTION__);
	tree_init(&tree_items[0].tree);
	tree_add_child(&tree_items[0].tree, &tree_items[1].tree, 1);
	tree_add_child(&tree_items[0].tree, &tree_items[2].tree, 1);
	tree_add_child(&tree_items[1].tree, &tree_items[3].tree, 1);
	tree_add_child(&tree_items[3].tree, &tree_items[4].tree, 1);

	//tree_remove_child(&tree_items[0].tree, &tree_items[1].tree);
	fprintf(stderr, "%s serach_item res:%p\n",
		__FUNCTION__, tree_search(&tree_items[0].tree, check_c, 1));

	fprintf(stderr, "%s item:%p\n",
		__FUNCTION__,
		find_gril_tree_item(tree_items, "a/c/d"));
	fprintf(stderr, "%s item:%p\n",
		__FUNCTION__,
		find_gril_tree_item(tree_items, "a/b"));
	fprintf(stderr, "%s item:%p\n",
		__FUNCTION__,
		find_gril_tree_item(tree_items, "b"));

	return 0;
}
#endif
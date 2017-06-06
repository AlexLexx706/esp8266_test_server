/** alexlexx 2017 */
#ifndef TREE_H
#define TREE_H
#include "list.h"

/**definition tree struct*/
typedef struct Tree_t {
	struct Tree_t * parent;
	List child_head;
	List head;
} Tree;

//check tree item function
typedef Tree * (*tree_check_fun)(Tree* item);
/*
	Init tree.
	params:
		tree - tree item;
*/
void tree_init(Tree * tree);

/*
	Add child tree to parent tree.
	params:
		tree - parant tree item;
		child - child tree item;
		reset_childs - 1 - reset childs list in child tree item, 9 -not reset
**/
int tree_add_child(Tree * tree, Tree * child, int reset_childs);

/*
	Remove child tree item from parrent tree.
	params:
		tree - parant tree item;
		child - child tree item;
**/
int tree_remove_child(Tree * tree, Tree * child);

/**
	search item in tree
	params:
		tree - parant tree item;
		tree_search_fun - function for check tree item;
	return:
		NULL or item
*/
Tree * tree_search(Tree * tree, tree_check_fun fun, int recursive);
#endif

/**alexlexx 2017*/
#ifndef CONTROLLER_H
#define CONTROLLER_H
#include "tree.h"

struct GrilTreeItem_t;
struct GrilStreamParcerResultSender_t;
struct GrilStreamParcerResult_t;
/**
	Gril item tree node.
	name - name of node
	fun_set - fun call for set cmd
	fun_print - fun call for print cmd
*/
typedef struct GrilTreeItem_t {
	const char * name;
	enum GrilStreamParcerError (* fun_set)(
		struct GrilTreeItem_t * item,
		const char * value);
	void (* fun_print)(
		struct GrilTreeItem_t * item,
		struct GrilStreamParcerResultSender_t * sender);
	Tree tree;
} GrilTreeItem;

/**
 	Init command controller with root node
 	params:
 		root - controller tree root none, use for search nodes in tree
*/
void controller_init(GrilTreeItem * root);

/**
	Process data from gril parcer, and return results.
	params:
		parcer_res - parcer result;
*/
void controller_process_commands(struct GrilStreamParcerResult_t * parcer_res);
#endif

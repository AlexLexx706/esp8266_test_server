/**alexlexx 2017*/
#ifndef CONTROLLER_H
#define CONTROLLER_H
#include "gril_stream_cmd_parcer.h"
#include "tree.h"

/**This struct use for send controller result back*/
typedef struct {
    void * socket;
    void (*send_data)(void *socket, char *string);

} Userdata;

//define gril item
struct GrilTreeItem_T;

//set gril item data fun definition
typedef enum GrilStreamCmdParcerError (*set_gril_item_fun)(
	struct GrilTreeItem_T * item, const char * value);

//print gril item fun definition
typedef enum GrilStreamCmdParcerError (*print_gril_item_fun)(
	struct GrilTreeItem_T * item, char ** out_buffer, const char * out_buffer_end);

//defeine gril item tree node
typedef struct GrilTreeItem_T 
{
	const char * name;
	set_gril_item_fun fun_set;
	print_gril_item_fun fun_print;
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
		error - parser result;
		cmd - cmd string;
		param - parametr string,
		value - value string;
		user_data - use for send result
*/
void controller_process_commands(
    enum GrilStreamCmdParcerError error,
    const char * prefix,
    const char * cmd,
    const char * param,
    const char * value,
    void * user_data);
#endif

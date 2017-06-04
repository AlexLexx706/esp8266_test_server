#ifndef CONTROLLER_H
#define CONTROLLER_H
#include "gril_stream_cmd_parcer.h"
#include "tree.h"

//User data struct definition
typedef struct Userdata {
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
	struct GrilTreeItem_T * item, char * out_buffer, int out_buffer_size);

//defeine gril item tree node
typedef struct GrilTreeItem_T 
{
	const char * name;
	set_gril_item_fun fun_set;
	print_gril_item_fun fun_print;
	Tree tree;
} GrilTreeItem;

//init controller;
void controller_init(GrilTreeItem * root);

//function for process commands
void controller_process_commands(
    enum GrilStreamCmdParcerError error,
    const char * prefix,
    const char * cmd,
    const char * param,
    const char * value,
    void * user_data);
#endif
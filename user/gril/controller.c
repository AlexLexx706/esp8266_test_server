#include "controller.h"
#include "gril_stream_parcer.h"
#include "assert.h"
#include "buffered_sender.h"

#ifndef LINUX
	#include "user_interface.h"
	#include "osapi.h"
#else
	#define LOCAL static
	#define ICACHE_FLASH_ATTR
	#define os_strcmp strcmp
	#define os_strncmp strncmp
	#define os_strlen strlen
	#define os_sprintf sprintf
	#include <stdio.h>
	#include <string.h>
#endif

LOCAL GrilTreeItem * root_gril_item = NULL;
LOCAL BufferedSender bf_sender;
LOCAL GrilStreamParcerResultSender bf_result_sender;
LOCAL GrilTreeItem * find_gril_tree_item(
	GrilTreeItem * head_item, const char * path);

/**
LOCAL enum GrilStreamParcerError root_set(
	struct GrilTreeItem_t * item, const char * value);
*/

LOCAL void print_tree(
	struct GrilTreeItem_t * item,
	struct GrilStreamParcerResultSender_t * sender);


void ICACHE_FLASH_ATTR
controller_init(GrilTreeItem * root) {
	assert(root);
	#ifdef DEBUG_CONTROLLER
	fprintf(stderr, "%s root:%p\n",  __FUNCTION__, root);
	#endif

	root_gril_item = root;
	tree_init(&root->tree);
	root->name = "/";
	root->fun_set = NULL;
	root->fun_print = print_tree;

	//init buffered sender
	bf_result_sender.sender = &bf_sender;
	bf_result_sender.fun_send = buffered_sender_send;
}

void ICACHE_FLASH_ATTR
controller_init_item(
        GrilTreeItem * parent,
        GrilTreeItem * item,
        const char * name) {
    assert(parent);
    assert(item);
    assert(name);
    tree_add_child(&parent->tree, &item->tree, 1);
	item->fun_print = print_tree;
	item->fun_set = NULL;
	item->name = name;
}

/**
LOCAL enum GrilStreamParcerError ICACHE_FLASH_ATTR
root_set(struct GrilTreeItem_t * item, const char * value) {
	#ifdef DEBUG_CONTROLLER
	fprintf(stderr, "%s item->name:%s value:%s\n",  __FUNCTION__, item->name, value);
	#endif
	return GrilStreamParcerNoError;
}
*/

LOCAL void ICACHE_FLASH_ATTR
print_tree(
		struct GrilTreeItem_t * head,
		struct GrilStreamParcerResultSender_t * sender) {
	assert(head);
	assert(sender);
	List * pos;
	GrilTreeItem * item;

	sender->fun_send(sender->sender, "{", 1);

	LIST_ITER(&(head->tree.child_head)) {
		item = LIST_ITEM (GrilTreeItem, tree.head, pos);
		sender->fun_send(sender->sender, item->name, strlen(item->name));
		sender->fun_send(sender->sender, "=", 1);
		//try print item value to out_buffer
		if (item->fun_print) {
			item->fun_print(item, sender);
		} else {
			sender->fun_send(sender->sender, "NULL", 4);
		}
		if (pos->next != &head->tree.child_head) {
            sender->fun_send(sender->sender, ",", 1);
        }
	}
	sender->fun_send(sender->sender, "}", 1);
}


LOCAL GrilTreeItem * ICACHE_FLASH_ATTR
find_gril_tree_item(GrilTreeItem * head_item, const char * path) {
	List * pos;
	GrilTreeItem * item;
	assert(path);
	assert(head_item);
	#ifdef DEBUG_CONTROLLER
	fprintf(stderr, "%s item:%s path:%s\n",
		__FUNCTION__, head_item->name, path);
	#endif

	const char * end = strchr(path, '/');


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

LOCAL void ICACHE_FLASH_ATTR
controller_send_res(
		struct GrilStreamParcerResult_t * parcer_res) {
	int len = os_strlen(parcer_res->prefix) + 2;
	char out_buffer[32];

	if (parcer_res->error == GrilStreamParcerNoError) {
		os_sprintf(out_buffer, "RE%03X%%%s%%\n", len, parcer_res->prefix);
		parcer_res->sender->fun_send(
			parcer_res->sender->sender,
			out_buffer,
			len + 6);
	} else {
		char error_buffer[8];
		len += os_sprintf(error_buffer, "%d", parcer_res->error);
		len = os_sprintf(
			out_buffer, "ER%03X%%%s%%%s\n",
			len, parcer_res->prefix, error_buffer);
		parcer_res->sender->fun_send(
			parcer_res->sender->sender,
			out_buffer,
			len);
	}
}

void ICACHE_FLASH_ATTR
controller_process_commands(struct GrilStreamParcerResult_t * parcer_res) {
	assert(root_gril_item != NULL);
	#ifdef DEBUG_CONTROLLER
	fprintf(stderr, "%s error:%d prefix:%s cmd:%s path:%s value:%s\n",
		__FUNCTION__, error,  prefix, cmd, path, value);
	#endif

	if (parcer_res->error == GrilStreamParcerNoError) {
		int path_len = os_strlen(parcer_res->param);
		if (path_len > 0) {
			GrilTreeItem * item = (path_len == 1 && parcer_res->param[0] == '/') ?
				root_gril_item : find_gril_tree_item(
					root_gril_item,
					parcer_res->param[0] == '/' ?
						&parcer_res->param[1]:
						parcer_res->param);
			if (item) {
				if (!os_strcmp(parcer_res->cmd, "set") && item->fun_set) {
					parcer_res->error = item->fun_set(item, parcer_res->value);
				} else if (!os_strcmp(parcer_res->cmd, "print") && item->fun_print) {
					buffered_sender_init(&bf_sender, parcer_res, 1);
					item->fun_print(item, &bf_result_sender);
					buffered_sender_flush(&bf_sender);
					return;
				} else {
					parcer_res->error = GrilStreamParcerErrorUnknownCmd;
				}
			} else {
				parcer_res->error = GrilStreamParcerErrorWrongParam;
			}
		}
	}
	controller_send_res(parcer_res);
}


#ifdef TEST_CONTROLLER
void send_data(void *socket, const char *string, int str_len) {
	fprintf(stderr, "%s res:%s", __FUNCTION__, string);
}

LOCAL void print_light_state(
	struct GrilTreeItem_t * item,
	struct GrilStreamParcerResultSender_t * sender) {
	assert(item);
	assert(sender);
    char buffer[4];
    int len = os_sprintf(buffer, "%d", 1);
    sender->fun_send(sender->sender, buffer, len);
}


LOCAL void print_light_value(
	struct GrilTreeItem_t * item,
	struct GrilStreamParcerResultSender_t * sender) {
	assert(item);
	assert(sender);
    char buffer[4];
    int len = os_sprintf(buffer, "on");
    sender->fun_send(sender->sender, buffer, len);
}


LOCAL void print_shm_state(
	struct GrilTreeItem_t * item,
	struct GrilStreamParcerResultSender_t * sender) {
	assert(item);
	assert(sender);
    char buffer[4];
    int len = os_sprintf(buffer, "ok");
    sender->fun_send(sender->sender, buffer, len);
}


int main() {
	GrilTreeItem root_gril;
	GrilStreamParcer parcer;
	GrilCommandNameDesc cmd_names[] = {{"print", 5}, {"set", 3}};
	GrilStreamParcerResultSender result_sender = {0, send_data};


	GrilTreeItem light_root;
	GrilTreeItem light_state;
	GrilTreeItem light_value;

	GrilTreeItem shm_root;
	GrilTreeItem shm_state;


	//char test_date[] = "%%print,/,on\npri\nset,/,12\n%12%\n";
	char test_date[] = "%%print,/\n%AB%print,/light/value\n";

	controller_init(&root_gril);
	controller_init_item(&root_gril, &light_root, "light");

	controller_init_item(&light_root, &light_state, "state");
	light_state.fun_print = print_light_state;

	controller_init_item(&light_root, &light_value, "value");
    light_value.fun_print = print_light_value;

	controller_init_item(&root_gril, &shm_root, "shm");
	controller_init_item(&shm_root, &shm_state, "state");
    shm_state.fun_print = print_shm_state;

	gril_stream_parcer_init(
		&parcer, controller_process_commands,
		&result_sender, cmd_names, 2);
	gril_stream_parcer_parce(&parcer, test_date, sizeof(test_date));
	fprintf(stderr, "%s handler:%p\n", __FUNCTION__, controller_process_commands);
	return 0;
}
#endif

#ifndef LIGHT_H
#define LIGHT_H

struct GrilTreeItem_t;
void light_init();

enum GrilStreamParcerError light_set(
	struct GrilTreeItem_t * item, const char * value);
void light_print(
		struct GrilTreeItem_t * item,
		struct GrilStreamParcerResultSender_t * sender);

#endif
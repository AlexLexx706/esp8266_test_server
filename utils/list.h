#ifndef LIST_H
#define LIST_H
	typedef struct List_t {
		struct List_t * next;
		struct List_t * prev;
	} List;

	//init list head
	#define INIT_LIST(head)\
		(head)->next = (head);\
		(head)->prev = (head);

	//return pointr to item 
	#define LIST_TO_ITEM(ItemType, head_member, pos) ((ItemType *)((void *)pos - (void *)(&((ItemType *)0)->head_member)))

	//add list item to tail of list
	void list_add_tail(List * head, List * item);

	//remove lit item from list
	int list_remove(List * item);

#endif
/** alexlexx 2017 */
#ifndef LIST_H
#define LIST_H
	/**two directions list definition*/
	typedef struct List_t {
		struct List_t * next;
		struct List_t * prev;
	} List;

	//init list head
	#define LIST_INIT(head)\
		(head)->next = (head);\
		(head)->prev = (head);

	//return pointr to item 
	#define LIST_ITEM(ItemType, head_member, pos) ((ItemType *)((char *)pos - (char *)(&((ItemType *)0)->head_member)))
	
	//define for head
	#define LIST_ITER(head) for (pos = (head)->next; pos != (head); pos = pos->next)

	//add list item to tail of list
	void list_add_tail(List * head, List * item);

	//remove lit item from list
	int list_remove(List * item);

#endif

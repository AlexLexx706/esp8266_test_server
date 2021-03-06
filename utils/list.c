#include "list.h"
#include "assert.h"

#ifdef TEST
	#include <stdio.h>
#endif


void list_add_tail(List * head, List * item) {
	assert(head);
	assert(item);
	item->next = head;
	item->prev = head->prev;
	head->prev->next = item;
	head->prev = item;
	#ifdef DEBUG_LIST
	fprintf(stderr, "h:%p h->p:%p h->n:%p i->p:%p i->n:%p\n", head, head->prev, head->next, item->prev, item->next);
	#endif
}

//remove lit item from list
int list_remove(List * item) {
	assert(item);
	if (item->next != item) {
		item->prev->next = item->next;
		item->next->prev = item->prev;
		return 0;
	}
	return 1;
}


#ifdef LIST_TEST

void main() {
	List head;
	LIST_INIT(&head);
	List item1;
	List item2;
	List item3;
	list_add_tail(&head, &item1);
	list_add_tail(&head, &item2);
	list_add_tail(&head, &item3);

	List * pos;
	int i = 0;
	for (pos = (&head)->next; pos != (&head); pos = pos->next) {
		i++;
	}
	fprintf(stderr, "count:%d\n", i);

	list_remove(&item2);
	
	i = 0;
	for (pos = (&head)->next; pos != (&head); pos = pos->next) {
		i++;
	}
	fprintf(stderr, "count:%d\n", i);

	typedef struct {
		const char * name;
		List head;
	} __attribute__ ((packed)) CmdData;

	List cmd_head;
	LIST_INIT(&cmd_head);

	CmdData data1 = {"1",};
	CmdData data2 = {"2",};
	fprintf(stderr, "data1:%p\ndata2:%p\n", &data1, &data2);
	fprintf(stderr, "sizeof CmdData:%zu\n", sizeof(CmdData));

	list_add_tail(&cmd_head, &data1.head);
	list_add_tail(&cmd_head, &data2.head);

	LIST_ITER(cmd_head) {
		CmdData * item = LIST_ITEM(CmdData, head, pos);
		fprintf(stderr, "name:%s\n", item->name);
	}


}

#endif
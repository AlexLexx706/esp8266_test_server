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
    #ifdef TEST
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


#ifdef TEST

void main() {
    List head;
    INIT_LIST(&head);
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

    struct  CmdData
    {
    	const char * name;
        List head;
    };

	List cmd_head;
	INIT_LIST(&head);

    struct CmdData data1;
    struct CmdData data2;
    list_add_tail(&cmd_head, &data1);
    list_add_tail(&cmd_head, &data2);


}

#endif
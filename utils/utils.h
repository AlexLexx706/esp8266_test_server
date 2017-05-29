#ifndef UTILS_H
#define UTILS_H
    void ftoa(double n, char *res, int afterpoint);

    typedef struct List_t {
        List_t * next;
        List_t * prev;
    } List;

    //init list head
    #define INIT_LIST(list)\
        list->next = list;\
        list->prev = list;

    //add list item to tail of list
    void list_add_tail(List * head, List * item);

    //remove lit item from list
    void list_remove(List * item);

#endif
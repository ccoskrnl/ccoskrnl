#ifndef __LIBK_LIST_H__
#define __LIBK_LIST_H__
#include "../types.h"



typedef struct _list_node {
    struct _list_node* flink;
    struct _list_node* blink;
} list_node;

void _list_init(list_node* list);
void _list_push(list_node* list, list_node* node);
void _list_remove_from_list(list_node* list, list_node* node);

#endif
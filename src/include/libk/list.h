#ifndef __LIBK_LIST_H__
#define __LIBK_LIST_H__
#include "../types.h"



typedef struct _list_node_t {
    struct _list_node_t* flink;
    struct _list_node_t* blink;
}__attribute__((packed)) list_node_t;

void _list_init(list_node_t* list);
void _list_push(list_node_t* list, list_node_t* node);
void _list_remove_from_list(list_node_t* list, list_node_t* node);

#endif
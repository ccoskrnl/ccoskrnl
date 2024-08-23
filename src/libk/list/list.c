#include "../../include/libk/list.h"


inline void _list_init(list_node* list)
{
    list->blink = NULL;
    list->flink = NULL;
}

inline void _list_push(list_node* list, list_node* node)
{
    if (list->flink == NULL)
    {
        list->flink = node;
        list->blink = node;
        node->blink = list;
        node->flink = 0;
    }
    else 
    {
        node->flink = 0;
        node->blink = list->blink;
        list->blink->flink = node;
        list->blink = node;
    }
    
}

void _list_remove_from_list(list_node* list, list_node* node)
{

    // the node is only one in list
    if (list->flink == node && list->flink == list->blink && node->flink == NULL) 
    {
        _list_init(list);
    }
    // the node is the last item in list
    else if (list->blink == node && list->flink != list->blink && node->flink == NULL) {
        node->blink->flink = NULL;
        list->blink = node->blink;
    }
    // the node is the first item in list
    else if (list->flink == node && node->flink != NULL)
    {
        list->flink = node->flink;
    }
    else 
    {
        node->flink->blink = node->blink;
        node->blink->flink = node->flink;
    }
    


    _list_init(node);
}

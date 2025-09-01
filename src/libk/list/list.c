#include "../../include/libk/list.h"


inline void _list_init(list_node_t* list)
{
    list->blink = NULL;
    list->flink = NULL;
}

inline void _list_push(list_node_t* list, list_node_t* node)
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

list_node_t* _list_pop(list_node_t* list)
{

    if (list->flink == NULL && list->blink == NULL)
    {
        return NULL;
    }

    list_node_t* removed_node = list->blink;
    if (list->blink != NULL)
    {

        if (list->flink == list->blink)
        {
            _list_init(list);
            return removed_node;
        }

        list->blink = removed_node->blink;
        if (list->blink != NULL)
        {
            // We ensured that the blink is not NULL.
            // We need to update the flink pointer of the new blink node.
            list->blink->flink = NULL;
        }
    }

    return removed_node;
}

/**
 * @brief Dequeue the front item from the list
 * 
 * @param list 
 * @return list_node_t* 
 */
list_node_t* _list_dequeue(list_node_t *list)
{
    list_node_t *front = list->flink;

    if (list->flink != NULL)
    {
        if (list->flink == list->blink)
        {
            _list_init(list);
            return front;
        }

        if (list->flink != list->blink)
        {
            list->flink = list->flink->flink;
            return front;
        }
    }

    return front;
}

void _list_remove(list_node_t* list, list_node_t* node)
{

    // the node is only one in list
    if (list->flink == node && list->flink == list->blink && node->flink == NULL) 
    {
        _list_init(list);
    }
    // the node is the last item in list
    else if (list->blink == node && list->flink != list->blink && node->flink == NULL) 
    {
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

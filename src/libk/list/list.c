#include "../../include/libk/list.h"
#include "../../include/libk/stdlib.h"

/*  
    @list_buggy.c
    struct _list_node list;
    list->flink = list;
    list->blink = list; 
*/

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

inline void _list_pop(list_node* list)
{
    list_node* node;

    if (list->blink == NULL)
    {
        return;
    }

    if (list->blink == list || list->flink == list)
    {
        return _list_init(list);
    }
    
    // 对应于链表只有一项
    else if (list->blink == list->flink)
    {
        node = list->flink;

        _list_init(node);
        return _list_init(list);
        
    }

    node = list->blink;
    list->blink = node->blink;
    list->blink->flink = NULL;
    _list_init(node);
    
}

void _list_remove_from_list(list_node* list, list_node* node)
{
    list_node* next;

    next = list->flink;

    if (next == 0)
    {
        return;
    }
    else if (next == list)
    {
        return;
    }

    while (next != node)
    {
        next = next->flink;

        if (next == 0)
        {
            return;
        }

        // 对应于链表环的情况
        else if (next == list)
        {
            return;
        }
        
    }

    // 如果找到对应节点，就移除
    next->flink = node->flink;

    _list_init(node);
}

inline void _list_remove(list_node* node)
{
    list_node* prev;
    list_node* next;

    prev = node->blink;
    next = node->flink;

    assert((prev != 0));

    // 对应于将要被移除的节点为该链表中唯一的节点
    if (prev->flink == prev->blink)
    {
        _list_init(prev);
    }
    // 自旋链表
    else if (prev == next)
    {
        prev->flink = NULL;
        prev->blink = NULL;
    }
    // 该节点为链表的最后一项
    else if (next == 0)
    {
        prev->flink = 0;
    }
    else
    {
        prev->flink = next;
        next->blink = prev;
    }

    // 清空
    _list_init(node);

    return ;
    
}
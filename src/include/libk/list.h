#ifndef __LIBK_LIST_H__
#define __LIBK_LIST_H__
#include "../types.h"



typedef struct _list_node_t {
    struct _list_node_t* flink;
    struct _list_node_t* blink;
}__attribute__((packed)) list_node_t;


/**
 * @brief Initialize a list.
 * 
 * @param list 
 */
void _list_init(list_node_t* list);

/**
 * @brief Dequeue the front item from the list
 * 
 * @param list 
 * @return list_node_t* 
 */
list_node_t* _list_dequeue(list_node_t *list);

/**
 * @brief Push a node to the rear of the list
 * 
 * @param list 
 * @param node 
 */
void _list_push(list_node_t* list, list_node_t* node);


/**
 * @brief Pop the back item from the list
 * 
 * @param list 
 * @return list_node_t* , the popped node
 */
list_node_t* _list_pop(list_node_t* list);

/**
 * @brief Remove a node from the list
 * 
 * @param list 
 * @param node 
 */
void _list_remove(list_node_t* list, list_node_t* node);

#endif
#ifndef __LIBK_LIST_H__
#define __LIBK_LIST_H__
#include "../types.h"

/**
 * Return offset of the member_name from struct_type base.
 * 
 * @param[in]   struct_type     Specific struct type.
 * @param[in]   member_name     The member name was defined in struct_type.
 * 
 * @retval      Offset of the member_name from struct_type base.     
*/
#define element_offset(struct_type, member_name)    ((uint64_t)(&((struct_type*)0)->member_name))

/**
 * Get base address of struct type.
 * 
 * @param[in]   struct_type     Specific struct type.
 * @param[in]   member_name     The member name was defined in struct_type.
 * @param[in]   member_ptr      The pointer that points member_name.
 * 
 * @retval      A pointer that points struct_type.
*/
#define struct_base(struct_type, member_name, member_ptr) \
    (struct_type*)((uint64_t)member_ptr - element_offset(struct_type, member_name))


typedef struct _list_node {
    struct _list_node* flink;
    struct _list_node* blink;
} list_node;

void _list_init(list_node* list);
void _list_push(list_node* list, list_node* node);
void _list_pop(list_node* list);
void _list_remove_from_list(list_node* list, list_node* node);
void _list_remove(list_node* node);

#endif
#ifndef __LIBK_RBTREE_H__
#define __LIBK_RBTREE_H__
/*

Red-Black Properties:

    1.  Every node is either red or black.

    2.  The root is black.

    3.  Every leaf(nil) is black.

    4.  If a node is red, then both its children are black.

    5.  For each node, all simple paths from th node to descendant
        leaves contain the same number of black nodes.

Black-Depth:

    Definition:

        1.  The black-depth of a node is a red-black tree refers to
            the number of black nodes encountered when traversing 
            from the root to that specific node.

        2.  In other words, it represents the number of black ancestors
            of a given node.
    
    Uniform Black-Depth:

        1.  One of the essential properties of red-black trees is that
            every path from the root to any of its descendant NIL nodes
            (i.e., leaf node) contains the same number of black nodes.

        2.  This unifomity ensures that the tree remains balanced and
            efficient for search operations.


*/

#include "../types.h"

typedef enum
{
    RED = 0,
    BLACK = 1
} RBTreeNodeColor;

typedef struct _rbtree_node
{
    uint64_t                                    key;

    RBTreeNodeColor                             color;

    struct _rbtree_node                         *left;

    struct _rbtree_node                         *right;

    struct _rbtree_node                         *parent;

} rbtree_node_t;


typedef void (*_rbtree_insert_t)(void* _this, rbtree_node_t* z);
typedef void (*_rbtree_delete_t)(void* _this, rbtree_node_t* z);
typedef rbtree_node_t* (*_rbtree_search_t)(void *_this, uint64_t key);

typedef struct _rbtree
{

    rbtree_node_t                               *root;
    rbtree_node_t                               *nil;
    rbtree_node_t                               node_nil;

    _rbtree_search_t                            Search;
    _rbtree_insert_t                            Insert;
    _rbtree_delete_t                            Delete;

} rbtree_t;

status_t new_a_rbtree(rbtree_t **rbtree);
status_t del_a_rbtree(rbtree_t *rbtree);

#endif
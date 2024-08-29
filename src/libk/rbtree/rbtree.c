#include "../../include/libk/rbtree.h"
#include "../../include/libk/stdlib.h"

static void left_rotate(void* _this, rbtree_node_t* x)
{
    rbtree_t* this = _this;

    // Set y to be x's right child.
    rbtree_node_t* y = x->right;


    // Turn y's left subtree into x's right subtree.
    x->right = y->left;

    // If y's left subtree is not empty
    if (y->left != this->nil) 
    {
        // x becomes the parent of the subtree's root.
        y->left->parent = x;
    }

    // Link x's parent to y
    y->parent = x->parent;

    // If x is the root of the tree
    if (x->parent == this->nil)
        this->root = y;
    else if (x == x->parent->left)
        x->parent->left = y;
    else
        x->parent->right = y;

    y->left = x;
    x->parent = y;

}

static void right_rotate(void* _this, rbtree_node_t* x)
{
    rbtree_t* this = _this;

    // Set y to be x's left child
    rbtree_node_t *y  = x->left;

    // Turn y's right subtree into x's left subtree.
    x->left = y->right;

    // If y's right subtree is not empty
    if (y->right != this->nil)
    {
        y->right->parent = x;
    }

    // Link x's parent to y
    y->parent = x->parent;

    // If x is the root of the tree
    if (x->parent == this->nil)
        this->root = y;
    else if (x == x->parent->left)
        x->parent->left = y;
    else
        x->parent->right = y;
    
    y->right = x;
    x->parent = y;

}

static rbtree_node_t* minimum(rbtree_node_t *x)
{
    while (x->left != NULL) 
        x = x->left;

    return x;
}

/**
 * The routine replaces the subtree rooted at node u by
 * the subtree rooted at node v.
 * */
static void transplant(void *_this, rbtree_node_t *u, rbtree_node_t *v)
{
    rbtree_t* this = _this;

    if (u->parent == this->nil)
        this->root = v;
    else if ( u == u->parent->left)
        u->parent->left = v;
    else
        u->parent->right = v;

    // The procedure can assign to v->parent even if v points to the sentinel.
    v->parent = u->parent;
}


/**
 * The routine determines which violations of the red-black
 * properties might arise in insert() upon inserting node z
 * and coloring it red. Moreover, considering the overall 
 * goal of the while loop. Finally, it explores each of the
 * three cases within the while loop's body.
 * */

static void insert_fixup(void* _this, rbtree_node_t* z)
{

    rbtree_t *this = _this;


    /**
    *  The routine enters a loop iteration only if z->parent is red
    *  so that z->parent cannot be the root(by property 2). Hence,
    *  z->parent->parent exists. 
    *  If z will be root of the tree, its parent is sentinil(color
    *  is black). Therefore, routine not enters iteration below at
    *  this time.
    * */

    while ( z->parent->color == RBTreeNodeRed) 
    {
        /*  Check if z's parent is a left child. */
        if (z->parent == z->parent->parent->left)
        {
            //  y is z's uncle
            rbtree_node_t *y = z->parent->parent->right;

            /** 
             * Check if both z's parent and uncle are red in color.

             * If y(z's uncle) is red, then case 1 is encountered.
             * Otherwise, control passes to cases 2 and 3. 

             * In all three cases, z's grandparent z->parent->parent
             * is black, since its parent z->parent is red.
             * */ 
            
            // property 4 is violated only between z and z->parent.
            if (y->color == RBTreeNodeRed) 
            {
                /* Case 1 */
                z->parent->color = RBTreeNodeBlack;

                y->color = RBTreeNodeBlack;
                z->parent->parent->color = RBTreeNodeRed;

                /* The will loop continues with node z's 
                   grandparent z->parent->parent as the new z. */
                z = z->parent->parent;
            }

            else
            {
                /* Case 2 */
                if (z == z->parent->right)
                {
                    /**
                     * Case 2 transforms into case 3 by a left
                     * rotation while preserves property 5.
                     * */
                    z = z->parent;
                    left_rotate(this, z);
                    
                }

                /* Case 3 */
                z->parent->color = RBTreeNodeBlack;

                z->parent->parent->color = RBTreeNodeRed;
                right_rotate(this, z->parent->parent);
            }

        }

        else 
        {
            rbtree_node_t *y = z->parent->parent->left;
            if (y->color == RBTreeNodeRed) 
            {
                z->parent->color = RBTreeNodeBlack;
                y->color = RBTreeNodeBlack;

                z->parent->parent->color = RBTreeNodeRed;
                z = z->parent->parent;
            }
            else
            {
                if (z == z->parent->left)
                {
                    z = z->parent;
                    right_rotate(this, z);
                }

                z->parent->color = RBTreeNodeBlack;
                z->parent->parent->color = RBTreeNodeRed;
                left_rotate(this, z->parent->parent);
            }
        }
    }

    // We ensure that the property 1 not be violated.
    this->root->color = RBTreeNodeBlack;
}


static void insert(void* _this, rbtree_node_t* z)
{
    rbtree_t *this = _this;

    // Node being compared with z
    rbtree_node_t *x = this->root;
    // y will be parent of z
    rbtree_node_t *y = this->nil;

    // Descend until reaching the sentinel.
    while (x != this->nil) 
    {
        y = x;

        if (z->key < x->key)
            x = x->left;
        else 
            x = x->right;
    }

    // Found the location - insert z with parent y
    z->parent = y;

    if ( y == this->nil) 
        // The rbtree is empty.
        this->root = z;
    else if (z->key < y->key)
        y->left = z;
    else
        y->right = z;

    // Both of z's children are the sentinel.
    z->left = this->nil;
    z->right = this->nil;

    // The new node starts out red.
    z->color = RBTreeNodeRed;

    /**
     * Because coloring z red may cause a violation of one
     * of the red-black properties, so we need to call
     * insert_fixup(this, z) in order to restore the
     * red-black properties.
     * */

    // Correct any violations of red-black properties.
    insert_fixup(this, z);

}

static void delete_fixup(void* _this, rbtree_node_t* x)
{
    rbtree_t *this = _this;

    // w is x's sibling node.
    rbtree_node_t *w;

    while ( x != this->root && x->color == RBTreeNodeBlack )
    {
        // Within the while loop, x always points to a nonroot
        // double black node.

        if (x == x->parent->left)
        {
            // Is x a left child?
            w = x->parent->right;

            /**
             * Case 1, x's sibling w is red. 
             * Thus, case 1 convert2 into one cases 2, 3 or 4.
             **/
            if (w->color == RBTreeNodeRed)
            {
                w->color = RBTreeNodeBlack;
                x->parent->color = RBTreeNodeRed;
                left_rotate(this, x->parent);
                w = x->parent->right;
            }

            /**
             * Case 2, x's sibling w is black, and both of w's
             * children are black.
             * Coloring node w red and setting node x to point
             * to x's parent. If case 2 enters through case 1
             * (the color of original x's parent is red), the
             * new node x is red-and-black(original x's parent
             * x->parent took on an extra black.), since the
             * original x->parent was red.*/
            if (w->left->color == RBTreeNodeBlack && w->right->color == RBTreeNodeBlack)
            {
                w->color = RBTreeNodeRed;

                /**
                 * To compensate for x and w each losing one
                 * black, x's parent x->parent can take on an
                 * extra black.
                 * Moving x up one level, so that the while loop
                 * repeats with x->parent as the new node x.
                 * (node x approaches the root of Red-Black Tree.)
                 **/
                x = x->parent;
            }

            else
            {
                /**
                 * Case 3. x's sibing w is black, w's left child is
                 * red, and w's right child is black.
                 **/
                if (w->right->color == RBTreeNodeBlack)
                {
                    // Switch the colors of w and its left child w->left
                    w->left->color = RBTreeNodeBlack;
                    w->color = RBTreeNodeRed;

                    // Perform a right rotation on w without violating
                    // any of the red-black properties.
                    right_rotate(this, x);

                    w = x->parent->right;
                }

                /**
                 * Case 4. x's sibling w is black, and w's right
                 * is red.
                 * Remove the extra black represented by x changing
                 * some colors and performing a left rotation 
                 * (without violating the red-black properties)
                 * and then the loop terminates in this case.
                 **/
                x->color = x->parent->color;
                x->parent->color = RBTreeNodeBlack;
                x->right->color = RBTreeNodeBlack;
                left_rotate(this, x->parent);
                x = this->root;
            }

        }

        else
        {
            w = x->parent->left;

            if (w->color == RBTreeNodeRed)
            {
                w->color = RBTreeNodeBlack;
                x->parent->color = RBTreeNodeRed;
                right_rotate(this, x->parent);
                w = x->parent->left;
            }
            if (w->right->color == RBTreeNodeBlack && w->left->color == RBTreeNodeBlack)
            {
                w->color = RBTreeNodeRed;
                x = x->parent;
            }
            else
            {
				if (w->left->color == RBTreeNodeBlack)
				{
					w->right->color = RBTreeNodeBlack;
					w->color = RBTreeNodeRed;
					left_rotate(this, w);
					w = x->parent->left;
				}

				w->color = x->parent->color;
				x->parent->color = RBTreeNodeBlack;
				w->left->color = RBTreeNodeBlack;
				right_rotate(this, x->parent);
				x = this->root;	
            }

        }

    }

    // Color the new node x (singly) black.
    x->color = RBTreeNodeBlack;

}

static void delete(void* _this, rbtree_node_t* z)
{

    /**
     * When the node z being deleted has at most one child,
     * then y will be z. When z has two children, then, y
     * will be z's successor, which has no left child and
     * moves into z's position in the tree. 
     * Additionally, y takes on z's color. In either case,
     * node y has at most one child: node x, which takes
     * y's place in the tree. (Node x will be the sentinel
     * if y has no children. 
     * */
    rbtree_t *this = _this;

    rbtree_node_t *y = z;

    
    rbtree_node_t *x = this->nil;
    RBTreeNodeColor y_original_color = y->color;

    /**
     * When the node z being deleted has at most one child,
     * then y will be z.
     * */
    if (z->left == this->nil)
    {
        x = z->right;

        // Replace z by its right child.
        transplant(this, z, z->right);
    }
    else if ( z->right == this->nil )
    {
        x = z->left;

        // Replace z by its left child.
        transplant(this, z, z->left);
    }
    
    /**
     * When z has two children, then, y will be z's successor, 
     * which has no left child and moves into z's position 
     * in the tree. And y and z are distinct. 
     * */
    else
    {
        // y is z's successor.
        y = minimum(z->right);
        y_original_color = y->color;

        x = y->right;

        // Is y farther down the tree?
        if (y != z->right)
        {
            // y may not has right child cause y->right is sentenil.
            transplant(this, y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }
        else
            x->parent = y;

        transplant(this, z, y);
        y->left = z->left;
        y->left->parent = y;

        y->color = z->color;

    }

    /**
     * If y was red, the red-black properties still hold when y is
     * removed or moved.
     **/
    
    /**
     * For property 5: Owing to the node y's color is BLACK, the
     * number of black-height of all ancestors containing node y
     * will descrase 1.
     * In this case, property 5 is violated. Since node x moves
     * into node y's original position, delete_fixup() proceduce
     * need to recolor x's sibling or one of its children.
     **/

    if (y_original_color == RBTreeNodeBlack)
        delete_fixup(this, x);
    
}

static rbtree_node_t* search(void *_this, uint64_t key)
{
    rbtree_t *this = _this;
    rbtree_node_t *x = this->root;
    while (x != this->nil && key != x->key) 
    {
        if (key < x->key)
            x = x->left;
        else
            x = x->right;
    }

    return x == this->nil ? NULL : x;
}

// status_t del_a_rbtree_node(rbtree_node_t *node)
// {
//     status_t status = ST_SUCCESS;

//     free(node);

//     return status;
// }

// status_t new_a_rbtree_node(rbtree_node_t **node)
// {
//     status_t status = ST_SUCCESS;

//     *node = (rbtree_node_t*)malloc(sizeof(rbtree_node_t));
//     if (*node == NULL) {
//         status = ST_FAILED;
//         return status;
//     }

//     (*node)->key = -1;
//     (*node)->color = RED;
//     (*node)->left = NULL;
//     (*node)->right = NULL;
//     (*node)->parent = NULL;

//     return status;
// }

// void destroy(rbtree_node_t *root, rbtree_node_t *nil)
// {
//     rbtree_node_t *left, *right, *prev;
//     left = root->left;
//     right = root->right;
//     if (left != nil) 
//     {
//         destroy(left, nil);
//     }
//     if (right != nil) 
//     {
//         destroy(right, nil);
//     }
//     free(left);
//     free(right);
// }

status_t del_a_rbtree(rbtree_t *rbtree)
{
    status_t status = ST_SUCCESS;

    // rbtree_node_t *left, *right;

    // destroy(rbtree->root, rbtree->nil);

    // if (rbtree->root == rbtree->nil) 
    //     free(rbtree->root);
    // else
    // {
    //     free(rbtree->root);
    //     free(rbtree->nil);
    // }

    free(rbtree);

    return status;
}

status_t new_a_rbtree(rbtree_t **rbtree)
{
    status_t status = ST_SUCCESS;

    *rbtree = (rbtree_t*)malloc(sizeof(rbtree_t));
    if (*rbtree == NULL) {
        status = ST_FAILED;
        return status;
    }
    (*rbtree)->node_nil.color = RBTreeNodeBlack;
    (*rbtree)->nil = &(*rbtree)->node_nil;
    (*rbtree)->root = (*rbtree)->nil;

    (*rbtree)->Search = search;
    (*rbtree)->Insert = insert;
    (*rbtree)->Delete = delete;
    return status;
}
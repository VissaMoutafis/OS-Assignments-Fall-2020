#pragma once

#include "Types.h"

typedef struct avl_node *AVLNode;
typedef struct avl_tree *AVL;

// AVL Methods

AVL avl_create(Compare compare, ItemDestructor itemDestructor);

bool avl_empty(AVL avl);

// classic bst search
AVLNode avl_find(AVL avl, Pointer key);

// classic tree insert with avl balance feature
void avl_insert(AVL avl, Pointer key);

// classic delete with delete entry switch
void avl_delete(AVL avl, Pointer key, bool delete_key, Pointer *old_key);

void avl_destroy(AVL avl);

// AVL Node methods
AVLNode avl_node_get_key(AVL avl, AVLNode node);
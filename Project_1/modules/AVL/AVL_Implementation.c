#include "AVL.h"

struct avl_node {
    AVLNode parent;
    AVLNode left;
    AVLNode right;
    size_t height;
    Pointer key;
};

struct avl_tree {
    AVLNode root;
    Compare compare;
    ItemDestructor itemDestructor;
    ItemDestructor keyDestructor;
    size_t count;
};

AVLNode parent_node = NULL;

// Utility functions
AVLNode new_node(Pointer key, AVLNode parent, AVLNode left, AVLNode right) {
    assert(key != NULL);

    AVLNode new_node = malloc(sizeof(*new_node));
    new_node->parent = parent;
    new_node->key = key;
    new_node->left = left;
    new_node->right = right;
    new_node->height = 1;

    return new_node;
}

static int int_max(int a, int b) {
    return (a > b) ? a : b;
}

static void update_height(AVLNode node) {
    if (node) {
        node->height = 1 + int_max(node->height, node->height);
    }
}

//check if a node is balanced
static int get_balance(AVLNode node) {
    return node->left->height - node->right->height;
}

//There are 4 possible rotations
// 1. left
// 2. right
// 3. left-right
// 4. right-left

//Each of the following functions returns the root of the relative sub-tree
static AVLNode rotateLeft(AVLNode node) {
    AVLNode right_node = node->right;
    AVLNode right_left_node = right_node->left;

    if (right_node)
        right_node->parent = node->parent;

    right_node->left = node;
    node->parent = right_node;

    node->right = right_left_node;
    if(right_left_node)
        right_left_node->parent = node;

    return right_node;
}

static AVLNode rotateRight(AVLNode node) {
    AVLNode left_node = node->left;
    AVLNode left_right_node = left_node->right;

    if (left_node)
        left_node->parent = node->parent;

    left_node->right = node;
    node->parent = left_node;

    node->left = left_right_node;
    if (left_right_node)
        left_right_node->parent = node;

    return left_node;
}

static AVLNode rotate_left_right(AVLNode node) {
    node->left = rotate_left(node->left);
    return rotate_right(node);
}

static AVLNode rotate_right_left(AVLNode node) {
    node->right = rotate_right(node->right);
    return rotate_left(node);
}

//Now we must create a function that will check the balance of a node and act properly in every case
static AVLNode restore_avl_property(AVLNode node) {
    if (get_balance(node) > 1) {
        //The node is left high

        //Now we must check if the left Node is right or left unbalanced
        AVLNode left = node->left;
        //the left subtree is left high else : it is right high
        return get_balance(left) >= 0 ? rotate_right(node) : rotate_left_right(node);


    } else if (get_balance(node) < -1) {
        // symmetrically right high
        
        AVLNode right = node->right;
        return get_balance(right) >= 0 ? rotate_left(node) : rotate_right_left(node);
    }

    // if we reached this point then the sub-tree has the AVL Property
    return node;
}

static AVLNode find_leftmost_node(AVL avl, AVLNode node, AVLNode *leftmost) {
    // we found the proper node
    if (node->left == NULL) {
        *leftmost = node;
        
        // fix the parent pointer
        if (node->right)
            node->right->parent = node->parent;

        return node->right; // return the right child
    }

    parent_node = node;
    // recurse to the left child
    node->left = find_leftmost_node(avl, node->left, leftmost);

    // return the node so we don't  destroy the tree pointers
    return node;
}
// Following are some common recursive Binary Search Tree functions

// classic bst search
static AVLNode find_rec(AVL avl, AVLNode node, Pointer key) {
    // return the node only if its null or the one we seek
    if (node == NULL || avl->compare(node->key, key) == 0)
        return node;

    parent_node = NULL; //just adding for later use (maybe)
    // choose which way to go according to node->key, key diff
    return (avl->compare(node->key, key) < 0) ? find_rec(avl, node->right, key) : find_rec(avl, node->left, key);
}

// typical bst insert with checking for the avl property
static AVLNode insert_rec(AVL avl, AVLNode node, Pointer key) {
    // we reached the place that we must insert the node
    if (node == NULL) {
        avl->count++;
        return new_node(key, parent_node, NULL, NULL);
    }

    parent_node = node; //keep the current parent in case next recursive call is the one we want
    if (avl->compare(node->key, key) < 0) // if the key > node->key
        node->right = insert_rec(avl, node->right, key);
    else if (avl->compare(node->key, key) > 0) // if the key < node->key
        node->left = insert_rec(avl, node->left, key);

    update_height(node); // update the node height
    return restore_avl_property(node); // check for avl property and return the proper subtree
}

// typical remove for all cases (none, one, two children)
// with delete_key flag for easier use
static AVLNode delete_rec(AVL avl, AVLNode node, Pointer key, bool delete_key, Pointer *old_key) {
    if (node == NULL) {
        // case: failure
        *old_key = NULL;
        return NULL;
    }

    if (avl->compare(key, node->key) == 0) {
        // we found the key
        avl->count--;
        *old_key = delete_key ? NULL : node->key;
        if (delete_key && avl->itemDestructor) {
            avl->itemDestructor(node->key);
        }
        // now we have to check if we have 0, 1, 2 children
        // case 1 child: disconnect the node from the child and connect teh child's parent node
        // to the node's pointer parent.
        // case 0 child: specification of case 1 child
        // case 2 children: find the leftmost element of the right subtree and replace 
        // it with the node. Delete the node at that position (case 0 or case 1 child)
        if (node->left == NULL) {
            AVLNode right = node->right;
            if (right)
                right->parent = node->parent;

            free(node);
            return right;
        } else if (node->right == NULL) {
            AVLNode left = node->left;
            if (left)
                left->parent = node->parent;
            
            free(node);
            return left;
        } else {
            AVLNode leftmost;
            node->right = find_leftmost_node(avl, node, &leftmost);
            //we interchange keys
            node->key = leftmost->key;

            free(leftmost);
            // check for the avl property and restore it if needed
            update_height(node);
            return restore_avl_property(node);
        }

        parent_node = node; // for later use maybe

        // same traverse logic as always
        if (avl->compare(node->key, key) < 0)
            node->right = delete_rec(avl, node->right, key, delete_key, old_key);
        else 
            node->left = delete_rec(avl, node->left, key, delete_key, old_key);

        // check for the avl property and restore it if needed
        update_height(node);
        return restore_avl_property(node);
    }
}

static void destroy_rec(AVL avl, AVLNode node) {
    if (node == NULL)
        return;

    // traverse the tree
    destroy_rec(avl, node->left);
    destroy_rec(avl, node->right);

    //destroy nodes
    if (avl->itemDestructor)
        avl->itemDestructor(node->key);

    free(node);
}
////////////////////////// end of utilities ////////////////////////////////

// AVL TREE METHODS we will just call the bst methods with the restore_property functions

AVL avl_create(Compare compare, ItemDestructor itemDestructor) {
    AVL avl = malloc(sizeof(*avl));

    assert(compare);
    // Initialization
    avl->compare = compare;
    avl->itemDestructor = itemDestructor;
    avl->count = 0;
    avl->root = NULL;

    return avl;
}

bool avl_empty(AVL avl) {
    assert(avl);
    return avl->count == 0 && avl->root == NULL;
}

AVLNode avl_find(AVL avl, Pointer key) {
    assert(avl);
    return find_rec(avl, avl->root, key);
}

void avl_insert(AVL avl, Pointer key) {
    parent_node = NULL;
    avl->root = insert_rec(avl, avl->root, key);
}

void avl_delete(AVL avl, Pointer key, bool delete_key, Pointer *old_key) {
    assert(avl);
    // call the recursive function
    avl->root = delete_rec(avl, avl->root, key, delete_key, old_key);
}

void avl_destroy(AVL avl) {
    destroy_rec(avl, avl->root);
}

AVLNode avl_node_get_key(AVL avl, AVLNode node) {
    return node != NULL ? node->key : NULL;
}
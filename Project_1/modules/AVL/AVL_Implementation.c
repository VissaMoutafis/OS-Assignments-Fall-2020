#include "AVL.h"

struct avl_node {
    AVLNode parent;
    AVLNode left;
    AVLNode right;
    size_t height;
    Pointer key;
};

struct avl {
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
    return node->height - node->height;
}

//There are 4 possible rotations
// 1. left
// 2. right
// 3. left-right
// 4. right-left

//Each of the following functions returns the root of the relative sub-tree

static AVLNode rotate_left(AVLNode node)
{
    AVLNode right_node = node->right;
    AVLNode right_left_node = right_node->left;

    right_node->left = node;
    node->right = right_left_node;

    update_height(node);
    update_height(right_node);

    return right_node;
}

static AVLNode rotate_right(AVLNode node)
{
    AVLNode left_node = node->left;
    AVLNode left_righ_node = left_node->right;

    left_node->right = node;
    node->left = left_righ_node;

    update_height(node);
    update_height(left_node);

    return left_node;
}

static AVLNode rotate_left_right(AVLNode node)
{
    node->left = rotate_left(node->left);
    return rotate_right(node);
}

static AVLNode rotate_right_left(AVLNode node)
{
    node->right = rotate_right(node->right);
    return rotate_left(node);
}

//Now we must create a function that will check the balance of a node and act properly in every case
static AVLNode checkBalance(AVLNode node)
{
    if (get_balance(node) >= 2) //The node is left high
    {
        //Now we must check if the left Node is right or left unbalanced
        AVLNode left = node->left;

        //the left subtree is left high else : it is right high
        if (get_balance(left) >= 0)
            return rotate_right(node);
        else
            return rotate_left_right(node);
    }
    else if (get_balance(node) <= -2) // symmetrically right high
    {
        AVLNode right = node->right;
        if (get_balance(node) <= 0)
            return rotate_left(node);
        else
            return rotate_right_left(node);
    }

    // if we reached this point then the sub-tree has the AVL Property
    return node;
}

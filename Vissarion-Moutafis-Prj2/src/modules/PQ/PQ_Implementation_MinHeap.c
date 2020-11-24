/*
** Implemented by Vissarion Moutafis
*/
#include <stdio.h>
#include "PQ.h"
#include <stdio.h>
struct pq {
    PQNode head;
    Compare compare;
    ItemDestructor itemDestructor;
    PQNode last_node;
    int len;
};

struct pq_node {
    Pointer entry;
    PQNode left, right, parent, prev;
};

PQNode new_node(Pointer entry, PQNode right, PQNode left, PQNode parent, PQNode prev) {
    PQNode node = malloc(sizeof(*node));
    assert(node);
    node->entry = entry;
    node->right = right;
    node->left = left;
    node->parent = parent;
    node->prev = prev;

    return node;
}
static void rec_tree_destroy(PQNode root, ItemDestructor itemdestructor) {
    if (root == PQ_EOF)
        return;
    
    rec_tree_destroy(root->left, itemdestructor);
    rec_tree_destroy(root->right, itemdestructor);
    if (itemdestructor)
        itemdestructor(root->entry);
    free(root);
}
// interchange the entries of the 2 nodes
static void exchange_nodes_entries(PQNode n1, PQNode n2) {
    Pointer tmp = n1->entry;
    n1->entry = n2->entry;
    n2->entry = tmp;
}

// return the node with the smallest entry
static PQNode get_min_node(PQNode n1, PQNode n2, Compare cmp) {
    if (!n1)
        return n2;
    else if (!n2)
        return n1;

    assert(n1 && n2);
    return cmp(n1->entry, n2->entry) < 0 ? n1 : n2;
}

static bool is_left_child(PQNode node) {
    assert(node);
    PQNode parent = node->parent;
    return parent && parent->left == node;
}

static bool is_right_child(PQNode node) {
    assert(node);
    PQNode parent = node->parent;
    return parent && parent->right == node;
}

static void insert_new_level(PQ pq, PQNode newnode) {
    PQNode cur = pq->head;
    while(cur->left) {
        cur = cur->left;
    }

    cur->left = newnode;
    newnode->parent = cur;
}

// static PQNode find_rightmost_leaf(PQ pq) {
//     assert(pq);
//     PQNode cur = pq->head;
//     assert(cur);
//     while (cur->right)
//         cur = cur->right;

//     return cur;
// }

// add a node to the end of the heap
static void add_last(PQ pq, Pointer entry) {
    PQNode newnode = new_node(entry, PQ_EOF, PQ_EOF, PQ_EOF, pq->last_node);
    PQNode last = pq->last_node;

    pq->last_node = newnode; // unique assignment so we don't bother later 

    if (last == PQ_EOF)
        // the tree is empty
        pq->head = newnode;
    else if (last == pq->head) {
        // case that tree hash only one node
        pq->head->left = newnode;
        newnode->parent = pq->head;
    } else {
        assert(last);
        assert(last->parent);
        // it's a leaf, not the root (it's got a parent)
        if (is_left_child(last)) {
            // if it's the left child just add the new node to the right
            last->parent->right = newnode;
            newnode->parent = last->parent;
        } else {
            PQNode parent = last->parent;
            while (is_right_child(parent))
                // while the parent is the right child and it has a parent
                parent = parent->parent; // set the parent pointer to be it's father

            assert(parent);

            if (parent == pq->head)
                insert_new_level(pq, newnode);
            else {
                parent = parent->parent->right;
            
                while (parent->left && parent->right)
                    parent = parent->left;

                // at this point the parent points to the actual parent of the last node
                // if the left child is empty then instert the node there else insert right
                if (parent->left == PQ_EOF)
                    parent->left = newnode;
                else
                    printf("ton hpiame\n");
                newnode->parent = parent;
            }
        }
    }
}
// static void recprint(PQNode r) {
//     if (!r) return;

//     recprint(r->left);
//     printf("%d\n", *(int*)r->entry);
//     recprint(r->right);
// }

// void printpq(PQ pq) {
    // recprint(pq->head);
// }
// remove the last node of the heap
static void remove_last(PQ pq) {
    // we need to remove the last node and then set a new node as the last one of the min-heap
    PQNode last = pq->last_node;
    if (last == PQ_EOF)
        return;
    else if (last == pq->head)
        pq->head = pq->last_node = PQ_EOF;
    else 
        pq->last_node = last->prev;
    
    // Now we dettach the last node
    if (last && last->parent) {
        if (is_right_child(last))
            last->parent->right = PQ_EOF;
        else 
            last->parent->left = PQ_EOF;
    }

    free(last);
}

static void heapify_up(PQ pq) {
    PQNode last = pq->last_node;
    if (!last)
        return;
    PQNode cur = last->parent;
    while (cur && pq->compare(last->entry, cur->entry) < 0) {
        // interchange the entries
        exchange_nodes_entries(cur, last);
        // proceed to pointer re-assignment
        last = cur;
        cur = cur->parent;
    }
}

static void heapify_down(PQ pq) {
    // perform heapify down
    PQNode node, cur;
    node = pq->head;
    if (!node)
        return;
    cur = get_min_node(pq->head->left, pq->head->right, pq->compare);
    while(cur && pq->compare(node->entry, cur->entry) >= 0) {
        exchange_nodes_entries(cur, node);
        node = cur;
        cur = get_min_node(cur->left, cur->right, pq->compare);
    }
}
// MIN PQ Methods

// create the min pqueue
PQ pq_create(Compare compare, ItemDestructor itemDestructor) {
    PQ pq = malloc(sizeof(*pq));
    assert(pq);
    assert(compare);

    pq->compare = compare;
    pq->itemDestructor = itemDestructor;
    pq->head = PQ_EOF;
    pq->len = 0;
    pq->last_node = PQ_EOF;

    return pq;
}

// no check for duplicates just pushing the item with its priority
void pq_push(PQ pq, Pointer entry) {
    // add the node as the last item of the complete tree
    add_last(pq, entry);
    heapify_up(pq);
    pq->len++;
}

// Popping the first item. If pq empty then return PQ_EOF
Pointer pq_pop(PQ pq) {
    // get the min element of pqueue
    PQNode min_node = pq->head;
    Pointer min_entry = min_node->entry;
    if(pq->head)
        pq->len--;
    // interchange the last and top node
    PQNode last = pq->last_node;
    exchange_nodes_entries(pq->head, last);
    // remove the last node since it is not needed
    remove_last(pq);

    // call heapify up to re sort the heap, since the root is the only node not 
    // fullfiling the heap feature
    heapify_down(pq);

    // and return the min value to the caller
    return min_entry;
}

// check if pq is empty
bool pq_empty(PQ pq) {
    assert(pq);
    return (pq->len == 0 && pq->head == PQ_EOF);
}

// deallocate the memory blocks that pq holds
void pq_destroy(PQ pq) {
    rec_tree_destroy(pq->head, pq->itemDestructor);
    free(pq);
}

// Utilities

void pq_set_destructor(PQ pq, ItemDestructor new_destructor) {
    assert(pq);
    pq->itemDestructor = new_destructor;
}

ItemDestructor pq_get_destructor(PQ pq) {
    assert(pq);
    return pq->itemDestructor;
}
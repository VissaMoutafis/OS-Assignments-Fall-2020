/*
** Implemented by Vissarion Moutafis
*/

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
    PQNode left, right, parent;
};

PQNode new_node(Pointer entry, PQNode right, PQNode left, PQNode parent) {
    PQNode node = malloc(sizeof(*node));
    assert(node);
    node->entry = entry;
    node->right = right;
    node->left = left;
    node->parent = parent;

    return node;
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

    return cmp(n1->entry, n2->entry) < 0 ? n1 : n2;
}

static bool is_left_child(PQNode node) {
    PQNode parent = node->parent;
    return parent && parent->left == node;
}

static bool is_right_child(PQNode node) {
    PQNode parent = node->parent;
    return parent && parent->right == node;
}

static void insert_new_level(PQ pq, PQNode newnode) {
    PQNode cur = pq->head;
    while(cur->left) {
        cur = cur->left;
    }

    cur->left = newnode;
}

// add a node to the end of the heap
static void add_last(PQ pq, Pointer entry) {
    PQNode newnode = new_node(entry, PQ_EOF, PQ_EOF, PQ_EOF);
    PQNode last = pq->last_node;
    pq->last_node = newnode;

    if (last == PQ_EOF) {
        // the tree is empty
        pq->head = newnode;
    } else if (last == pq->head) {
        // case that tree hash only one node
        pq->head->left = newnode;
    } else {
        // it's a leaf, not the root (it's got a parent)
        if (is_left_child(last)) {
            last->parent->right = newnode;
        } else {
            PQNode parent = last->parent;
            while (parent->parent && is_right_child(parent)) {
                // while the parent is the right child and it has a parent
                parent = parent->parent; // set the parent pointer to be it's father
            }
            
            if (parent == pq->head)
                insert_new_level(pq, newnode);
            else
                parent = parent->parent->right;
            
            while (parent->right) {
                parent = parent->left;
            }
            // at this point the parent points to the actual parent of the last node
            // if the left child is empty then instert the node there else insert right
            if (parent->left == PQ_EOF) {
                parent->left = newnode;
            } else {
                parent->right = newnode;
            }
        }
    }

}
// remove the last node of the heap
static void remove_last(PQ pq) {

}

static void heapify_up(PQ pq) {
    PQNode last = pq->last_node;

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
    // we will make last node the root of the min-heap
    PQNode last = pq->last_node;
    exchange_nodes_entries(pq->head, last);
    
    // remove the last node since it is not needed
    remove_last(pq);

    // perform heapify down
    PQNode node = pq->head, cur = get_min_node(pq->head->left, pq->head->right, pq->compare);

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
}

// Popping the first item. If pq empty then return PQ_EOF
Pointer pq_pop(PQ pq) {
    // get the min element of pqueue
    PQNode min_node = pq->head;
    Pointer min_entry = min_node->entry;

    // call heapify up to re sort the heap
    heapify_down(pq);

    //free the previous head
    free(min_node);
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
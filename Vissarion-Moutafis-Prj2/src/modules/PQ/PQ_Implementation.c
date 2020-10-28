#include "PQ.h"
#include <stdio.h>
struct pq {
    PQNode head;
    Compare compare;
    ItemDestructor itemDestructor;
    int len;
};

struct pq_node {
    Pointer entry;
    PQNode next;
};

PQNode new_node(Pointer entry, PQNode next) {
    PQNode node = malloc(sizeof(*node));
    assert(node);
    node->entry = entry;
    node->next = next;

    return node;
}

void insert_sorted(PQ pq, Pointer entry) {
    PQNode prev=NULL, cur = pq->head;

    // travel till you reach the end, or a node that is 
    while(cur != PQ_EOF && pq->compare(entry, cur->entry) > 0) {
        prev = cur;
        cur = cur->next;
    }
    if (prev == NULL)
        pq->head = new_node(entry, cur);
    else
        prev->next = new_node(entry, cur);
}

// PQ Methods

// create the pqueue
PQ pq_create(Compare compare, ItemDestructor itemDestructor) {
    assert(compare);

    PQ pq = malloc(sizeof(*pq));
    assert(pq);

    pq->compare = compare;
    pq->itemDestructor = itemDestructor;
    pq->head = PQ_EOF;
    pq->len = 0;

    return pq;
}

// no check for duplicates just pushing the item with its priority
void pq_push(PQ pq, Pointer entry) {
    assert(pq);
    if (pq_empty(pq)) {
        pq->head = new_node(entry, PQ_EOF);
    } else {
        insert_sorted(pq, entry);
    }
    pq->len++;
}

// Popping the first item. If pq empty then return PQ_EOF
Pointer pq_pop(PQ pq) {
    assert(pq);
    Pointer entry = NULL;
    if (!pq_empty(pq)) {
        pq->len--;
        PQNode poped = pq->head;
        
        // update the pq head;
        pq->head = poped->next;

        // get the entry
        entry = poped->entry;
        // free the memory
        free(poped);
    }

    // return the entry (if the pq is empty entry = NULL, else pq = head->entry)
    return entry;
}

// check if pq is empty
bool pq_empty(PQ pq) {
    assert(pq);
    return pq->head == PQ_EOF && pq->len == 0;
}

// deallocate the memory blocks that pq holds
void pq_destroy(PQ pq) {
    assert(pq);
    PQNode cur, next;
    cur = pq->head;
    // iterate till the end of the pqueue
    while(cur != PQ_EOF) {
        // keep the next node
        next = cur->next;

        // destroy the related memory
        if (pq->itemDestructor) 
            pq->itemDestructor(cur->entry);
        
        free(cur);

        // continue to the next node
        cur = next;
    }
    free(pq);
}

void pq_set_destructor(PQ pq, ItemDestructor new_destructor) {
    assert(pq);
    pq->itemDestructor = new_destructor;
}

ItemDestructor pq_get_destructor(PQ pq) {
    assert(pq);
    return pq->itemDestructor;
}
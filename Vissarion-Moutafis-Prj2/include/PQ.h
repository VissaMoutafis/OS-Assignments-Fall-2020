#pragma once

#include "Types.h"

typedef struct pq *PQ;
typedef struct pq_node *PQNode;

#define PQ_EOF NULL

// MIN PQ Methods

// create the min pqueue
PQ pq_create(Compare compare, ItemDestructor itemDestructor);

// no check for duplicates just pushing the item with its priority
void pq_push(PQ pq, Pointer entry);

// Popping the first item. If pq empty then return PQ_EOF
Pointer pq_pop(PQ pq);

// check if pq is empty
bool pq_empty(PQ pq);

// deallocate the memory blocks that pq holds
void pq_destroy(PQ pq);

// Utilities

void pq_set_destructor(PQ pq, ItemDestructor new_destructor);

ItemDestructor pq_get_destructor(PQ pq);

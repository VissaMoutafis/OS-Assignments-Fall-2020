#include <stdio.h>
#include "Types.h"
#include "PQ.h"
#define TEST_SIZE 10

void* make_int(int a) {
    int* _a = malloc(sizeof(int));
    *_a = a;
    return (void*)_a;
}

void destroy(Pointer a) {
    free(a);
}

int compare(Pointer a, Pointer b) {
    return *((int*)a) - *((int*)b); 
}

int main(void) {
    PQ pq = pq_create(compare, destroy);

    for (int i = 0; i < TEST_SIZE; i++)
        pq_push(pq, make_int(TEST_SIZE-i));

    printpq(pq);
    while(!pq_empty(pq)) {
        Pointer p = pq_pop(pq);
        printf("%d ", *((int*)p));
        free(p);
    }

    printf("\n");

    for (int i = 0; i < 2*TEST_SIZE; i++)
        pq_push(pq, make_int(2*TEST_SIZE-i));

    while(!pq_empty(pq)) {
        Pointer p = pq_pop(pq);
        printf("%d ", *((int*)p));
        free(p);
    }

    printf("\n");

    for (int i = 0; i < TEST_SIZE; i++)
        pq_push(pq, make_int(TEST_SIZE - i));
    
    pq_destroy(pq);
}
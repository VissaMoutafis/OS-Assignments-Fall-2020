#include <stdlib.h>
#include <assert.h>
#include "Stack.h"
struct stack_node
{
    Pointer entry;
    StackNode next;
};

struct stack
{
    StackNode head;
    int len;
    Compare compare;
    ItemDestructor itemDestructor;
};

Pointer stackNode_get_item(StackNode node) {
    return node ? node->entry : NULL;
}
StackNode create_stackNode(Pointer entry, StackNode next) {
    StackNode n = malloc(sizeof(*n));
    n->entry = entry;
    n->next = next;
    return n;
}

Stack stack_create(Compare compare, ItemDestructor itemDestructor) {
    Stack s = malloc(sizeof(*s));
    s->head = STACK_EOF;
    s->len = 0;

    return s;
}

bool stack_empty(Stack stack) {
    assert(stack);
    return stack->head == STACK_EOF && stack->len == 0;
}

StackNode stack_get_first(Stack stack) {
    assert(stack);
    return stack->head;
}

Pointer stack_pop(Stack stack) {
    assert(stack);
    if (stack_empty(stack))
        return NULL;
    
    StackNode new_head = stack->head->next;
    Pointer entry = stack->head->entry;
    free(stack->head);
    stack->len --;
    stack->head = new_head;

    return entry;
}

void stack_push(Stack stack, Pointer entry) {
    assert(stack);
    StackNode new_node = create_stackNode(entry, stack->head);
    stack->head = new_node;
    stack->len ++;
}

void stack_destroy(Stack *stack) {
    assert(stack);
    assert(*stack);

    StackNode next, n = (*stack)->head;
    
    while (n) {
        next = n->next;
        if ((*stack)->itemDestructor)
            (*stack)->itemDestructor(n->entry);
        free(n);
        n = next;
    }
    free(*stack);
    *stack = NULL;
}

int stack_len(Stack stack) {
    assert(stack);
    return stack->len;
}
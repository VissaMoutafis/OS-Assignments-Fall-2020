#pragma once

#include "Types.h"

typedef struct stack *Stack;
typedef struct stack_node *StackNode;

#define STACK_EOF (StackNode)0
#define STACK_BOF (StackNode)0

//Stack Basic Methods:
Stack stack_create(Compare compare, ItemDestructor itemDestructor);

// return true if stack is empty, false otherwise 
bool stack_empty(Stack stack);

// pop item from stack, return item, or NULL if stack is empty
Pointer stack_pop(Stack stack);

// return first item, or NULL if stack is empty
StackNode stack_get_first(Stack stack);

// push the entry to the stack (stack does not check if duplicate)
void stack_push(Stack stack, Pointer entry);

// de allocate the stack
void stack_destroy(Stack *stack);

// get the number of the elements in the stack
int stack_len(Stack stack);

//Some basic functionalities for a stack node
Pointer stackNode_get_item(StackNode node);

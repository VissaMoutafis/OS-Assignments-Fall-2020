#pragma once 

#include "Types.h"

typedef struct listnode* ListNode;
typedef struct list* List;

#define LIST_EOF ((ListNode)0)

//List methods

// Creates a list.
List list_create(Compare compare, ItemDestructor itemDestructor);

// Checks if the list is empty
bool list_empty(List list);

// Get the first entry of the list
ListNode list_get_head(List list);

// Finds the node and returns it, otherwise returns NULL.
ListNode list_find(List list, Pointer entry);

// Inserts an entry in the list.
// If the last is false it is inserted at the beginning, else it is inserted at the end.
void list_insert(List list, Pointer entry, bool last);

// Deletes the nodes and if the flag is TRUE then it destroys the entry, otherwise the entry is returned to the old_entry pointer.
// Returns true if the deletion was successful, otherwise return false.
// If last is false it deletes the first entry, else it deletes the last entry
bool list_delete(List list, Pointer entry, bool last, bool delete_entry, Pointer *old_entry);

// Sets a new itemDestructor for the list.
void list_set_destructor(List list, ItemDestructor itemDestructor);

// Return the 
ItemDestructor list_get_destructor(List list);

// De-allocate the memory allocated for the list.
void list_destroy(List *list);

// List Node methods

// Getter for the node entry
Pointer list_node_get_entry(List list, ListNode node);

// Set a new entry for a specific node.
void list_node_set_entry(List list, ListNode node, Pointer new_entry, Pointer *old_entry);
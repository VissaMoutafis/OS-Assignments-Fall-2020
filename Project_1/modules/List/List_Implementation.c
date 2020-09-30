/*
** Double Linked List Implementation
*/

#include "List.h"

struct listnode {
    Pointer entry;
    ListNode next;
    ListNode prev;
};

struct list {
    ListNode head;
    ListNode tail;
    size_t count;
    ItemDestructor itemDestructor;
    Compare compare;
};

ListNode create_node(Pointer entry, ListNode next, ListNode prev) {
    ListNode n = malloc(sizeof(*n));
    n->entry = entry;
    n->next = next;
    n->prev = prev;

    return n;
}


//List methods

// Creates a list.
List list_create(Compare compare, ItemDestructor itemDestructor) {
    assert(compare);
    
    List l = malloc(sizeof(*l)); // allocate the proper memory
    
    // Initialization
    l->count = 0;
    l->head = LIST_EOF;
    l->tail = LIST_EOF;
    l->compare = compare;
    l->itemDestructor = itemDestructor;

    return l;
}

// Checks if the list is empty
bool list_empty(List list) {
    assert(list);
    return list->count == 0 && list->head == LIST_EOF && list->tail == LIST_EOF;
}

// Get the first entry of the list
ListNode list_get_head(List list) {
    assert(list);
    return list->head;
}

// Finds the node and returns it, otherwise returns NULL.
ListNode list_find(List list, Pointer entry) {
    assert(list);

    ListNode cur = list_get_head(list);

    // search for the node
    while (cur != LIST_EOF) {
        // save the current entry
        Pointer cur_entry = list_node_get_entry(list, cur);
        
        // compare the entry with the current entry and
        // if its the entry we are looking for then return 
        // the node that 'cur' points to 
        if (compare(cur_entry, entry) == 0)
            return cur;
    }

    // if we reached this far then the search was unsuccessful
    return LIST_EOF;
}

// Inserts an entry in the list.
// If the last is false it is inserted at the beginning, else it is inserted at the end.
void list_insert(List list, Pointer entry, bool last) {
    /*
        The logic is the following:
        1. if last is true then insert at the end in O(1)
        2. if last is false then isert at the start  in O(1)
    */
    assert(list);
    ListNode new_node = create_node(entry, NULL, NULL);

    if (list_empty(list)) {
        // if the list is empty
        list->head = list->tail = new_node;

    } else {
        // if the list is not empty

        if (last) {
            // insert at the end
            list->tail->next = new_node;
            new_node->prev = list->tail;
            list->tail = new_node; 

        } else {
            // insert at the beginning
            list->head->prev = new_node;
            new_node->next = list->head;
            list->head = new_node; 
        }
    }
    list->count += 1;
}

// Delete a specific entry 
bool list_delete(List list, Pointer entry,  bool delete_entry, Pointer *old_entry) {
    assert(list);
    
    ListNode node = list_find(list, entry);
    if (node) {
        *old_entry = delete_entry == false ? list_node_get_entry(list, node) : NULL;
        ListNode prev, next;
        prev = node->prev;
        next = node->next;

        // detach the node from the list
        if (next)
            next->prev = prev;
        if (prev)
            prev->next = next;

        // make sure that the head and tail pointers are correctly adjusted 
        if (node == list->head)
            list->head = node->next;
        if (node == list->tail)
            list->tail = node->prev;

        // delete the node and the node's entry if it's ordered by the caller
        if (delete_entry) {
            if (list->itemDestructor)
                list->itemDestructor(node->entry);

            free(node);
        }

        list->count -= 1;
        assert(list->count >= 0); // debug line
        return true;
    }
    
    // if we reached this far something failed
    return false;
}

// Sets a new itemDestructor for the list.
void list_set_destructor(List list, ItemDestructor itemDestructor) {
    assert(list);
    list->itemDestructor = itemDestructor;
}

// Return the
ItemDestructor list_get_destructor(List list) {
    assert(list);
    return list->itemDestructor;
}

// De-allocate the memory allocated for the list.
void list_destroy(List *list) {
    ListNode cur, next;
    cur = (*list)->head;

    while (cur) {
        //make sure you keep the next node to traverse
        next = cur->next;

        // free the memory
        if ((*list)->itemDestructor)
            (*list)->itemDestructor(cur->entry);
        free(cur);

        // get to the next node
        cur = next;
    }

    free(*list);
    *list = NULL;
}

// List Node methods

// Getter for the node entry
Pointer list_node_get_entry(List list, ListNode node) {
    return node != NULL ? node->entry : NULL;
}

// Set a new entry for a specific node.
void list_node_set_entry(List list, ListNode node, Pointer new_entry, Pointer *old_entry) {
    assert(node);
    assert(list);
    assert(new_entry);
    assert(old_entry);
    *old_entry = node->entry;
    node->entry = new_entry;
}
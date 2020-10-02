/*  
**  Hash Table Implementation
**  Using Open Addressing with Double Hashing
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h> // for memcpy
#include <assert.h>
#include "HT.h"

struct hashtable {
    List *array;                // the array that holds the keys
    Compare compare;               // compare function for the searching
    Hash_Func hash;                // the hash function
    ItemDestructor itemDestructor; // the item destructor
    size_t size;                   // current size of the table
    size_t item_count;             // current count of the keys in the table
};
int cnt = 0;
// define if a slot is empty or not (trivial)
static bool is_empty_slot(List table_entry) {
    return table_entry == NULL;
}

// NOT NEEDED// Hash table rehashing function
// static void rehash(HT hash_table) {
//     // Create the new array
//     size_t old_size = hash_table->size;
//     size_t new_size = 2 * old_size;
//     // set the array pointers
//     Pointer *old_array = hash_table->array;
//     Pointer *new_array = calloc(new_size, sizeof(Pointer)); //initialize every block with NULL
//     // set the new table of the hash table
//     hash_table->array = new_array;
//     // Set the new size
//     hash_table->size = new_size;
//     // Copy the old to the new larger array
//     for (size_t i = 0; i < old_size; ++i) {
//         // for every element in the previous hash table, insert it to the new one
//         if (!is_empty_slot(old_array[i]))
//             ht_insert(hash_table, old_array[i]);
//     }
//     // free the memory of the old array
//     free(old_array);
// }

static bool find_key(List *array, size_t size, Pointer key, size_t key_hash, ListNode *entry_node, List *table_list) {
    assert(array != NULL);

    *entry_node = NULL;
    *table_list = NULL;

    // check if the slot is empty
    if (!is_empty_slot(array[key_hash])) {
        *table_list = array[key_hash];
        // Now we know the table index and we must find ListNode
        ListNode node = list_find(array[key_hash], key);
        // get the entry_node
        *entry_node = node;
        return (*entry_node != NULL);
    }
    
    // if we reached this far then we screw up
    return false;
}

static size_t calculate_ht_size(size_t entries) {
    // According to von Misse paradox (great theorem):
    // If 23 people are in a room then,
    // 2 of them has >50% chance of having the same birthday.
    // If we apply that with interpreting same birthday as a collision
    // then we get the following.
    return entries*365/22 ;
}

// Hash Table Methods Implementation

HT ht_create(Compare compare, Hash_Func hash_func, ItemDestructor itemDestructor, size_t max_entries) {
    HT ht = malloc(sizeof(*ht));

    ht->hash = hash_func;
    ht->compare = compare;
    ht->itemDestructor = itemDestructor;
    ht->size = max_entries > 0 ? calculate_ht_size(max_entries) : INITIAL_HT_SIZE;  //make sure that the size is a positive integer large enough
    ht->item_count = 0;

    // The table will be a 1-D array of Pointer* ( aka (void*)* )
    ht->array = calloc(ht->size, sizeof(List));

    return ht;
}

void ht_insert(HT hash_table, Pointer key) {
    // First hash the entry and find a suitable probe step
    size_t key_hash = hash_table->hash(key) % hash_table->size;
    ListNode entry_node = NULL;
    List table_list = NULL;
    Pointer old_entry=NULL;

    //Now we must add the key:
    
    // If the key is already in the table then we change the entry
    if (find_key(hash_table->array, hash_table->size, key, key_hash, &entry_node, &table_list)) {
        // set the new entry to the entry node 
        list_node_set_entry(hash_table->array[key_hash], entry_node, key, &old_entry);
        
        //delete the old entry
        if (hash_table->itemDestructor)
            hash_table->itemDestructor(old_entry);

    } else {
        //case that the key is not already in the hash table
        // so we must add it and increase the item_count

        if (is_empty_slot(hash_table->array[key_hash]))
            // if the slot is empty (there is no list there) then add a list and ... 
            hash_table->array[key_hash] = list_create(hash_table->compare, hash_table->itemDestructor);
        else cnt++;
        // add the key to the list
        list_insert(hash_table->array[key_hash], key, true); // add the entry at the end of the list
        // increase the item count
        hash_table->item_count++;
    }

    //NOT NEEDED // Check if there's a need for re-hashing
    // // if the item count surpasses half the size of the table then we need to rehash it
    // if (hash_table->size < 2 * hash_table->item_count)
    //     rehash(hash_table);
}

bool ht_contains(HT hash_table, Pointer key, Pointer *key_ptr)
{
    // First hash the entry and find a suitable probe step
    size_t key_hash = hash_table->hash(key) % hash_table->size;
    ListNode entry_node = NULL;
    List table_list = NULL;
    *key_ptr = NULL;
    
    // Now we must find the entry
    
    // we will use the find_key function
    if (find_key(hash_table->array, hash_table->size, key, key_hash, &entry_node, &table_list)) {
        // since we found the key  we must set the *key_ptr and return true
        *key_ptr = list_node_get_entry(hash_table->array[key_hash], entry_node);
        return true;
    }

    // we did not find the key
    return false;
}

void ht_delete(HT hash_table, Pointer key, bool delete_key, Pointer *key_ptr) {
    // First hash the entry and find a suitable probe step
    size_t key_hash = hash_table->hash(key) % hash_table->size;
    ListNode entry_node = NULL;
    List table_list = NULL;
    *key_ptr = NULL;

    if (find_key(hash_table->array, hash_table->size, key, key_hash, &entry_node, &table_list))
    {
        // we found the key => we delete the entry node but we don't yet delete the key
        list_delete(hash_table->array[key_hash], key, false, key_ptr);

        // we consider the delete_key flag to wheher delete the key or not.
        if (delete_key == true && hash_table->itemDestructor != NULL) { 
            // if we wanna delete the key
            // destroy the key
            hash_table->itemDestructor(*key_ptr);
            *key_ptr = NULL;
        }
        
        hash_table->item_count --;
    }
    else // we did not find the key
        *key_ptr = NULL;
}

// // helping function
// static void ht_print_keys(HT hash_table, Visit visit_key) {
//     for (size_t i = 0; i < hash_table->size; ++i) {
//         // if the slot is not empty then visit the key and display it.
//         // The diplay manner is determined by the caller
//         if (!is_empty_slot(hash_table->array[i]))
//             visit_key(hash_table->array[i]);
//     }
// }


// Simple function to free the memory
void ht_destroy(HT hash_table)
{
    // printf("collisions / item_count = %.1f", (float)cnt);
    for (size_t i = 0; i < hash_table->size; ++i) {
        // if the list-slot is not empty then free the memory
        if (!is_empty_slot(hash_table->array[i]))
            list_destroy(&(hash_table->array[i]));
    }
    free(hash_table->array);
    free(hash_table);
}
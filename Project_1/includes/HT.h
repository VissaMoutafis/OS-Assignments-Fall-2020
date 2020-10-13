#pragma once

#include "Types.h"
#include "List.h"

typedef struct hashtable* HT;
size_t config_size;
// Hash Table Methods

// Creates a Hash Table
// compare must return 0 if the 2 keys are the same and anything else otherwise
HT ht_create(Compare compare, Hash_Func hash_func, ItemDestructor itemDestructor, size_t max_entries);

//Adds a key to the hash table 
void ht_insert(HT hash_table, Pointer key);

// Find a key in the hash table
// If it exists then return true, else return false
// Return the Pointer key in the key pointer (key_ptr)
bool ht_contains(HT hash_table, Pointer key, Pointer *key_ptr);


// Delete an entry from the table
// If the delete_key flag is true then we delete the key and key_ptr == NULL
// Other wise the key is stored in the key+ptr pointer
void ht_delete(HT hash_table, Pointer key, bool delete_key, Pointer *key_ptr);

// NOT NEEDED// Function that will print all the keys in a specified-by-the-user way.
// void ht_print_keys(HT hash_table, Visit visit_key);

// de-allocates the memory allocated for the hash table
void ht_destroy(HT hash_table);
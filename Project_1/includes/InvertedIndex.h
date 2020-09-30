#pragma once

#include "Types.h"
#include "AVL.h"
#include "List.h"
typedef struct inverted_index* InvertedIndex;

// Inverted Index D.S. Methods

// Constructor Function
InvertedIndex invidx_create(Compare index_compare, ItemDestructor index_itemDestructor, Compare compare, ItemDestructor itemDestructor);

// Simple student insertion function 
void invidx_insert(InvertedIndex invidx, Pointer student);

// Delete a student from the index
// if delete_entry == true then we free the student struct memory
// otherwise we will keep the record at the old_entry pointer
void invidx_delete(InvertedIndex invidx, Pointer Student, bool delete_entry, Pointer *old_entry);


void invidx_destroy(InvertedIndex invidx);
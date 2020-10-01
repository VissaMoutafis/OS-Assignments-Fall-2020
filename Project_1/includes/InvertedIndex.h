#pragma once

#include "Types.h"
#include "List.h"


typedef struct inverted_index* InvertedIndex;
typedef struct index_struct* Index;

// Inverted Index D.S. Methods

// Constructor Function
InvertedIndex invidx_create(Compare compare, ItemDestructor itemDestructor);

// Simple student insertion function 
void invidx_insert(InvertedIndex invidx, Pointer student);

// Delete a student from the index
// if delete_entry == true then we free the student struct memory
// otherwise we will keep the record at the old_entry pointer
void invidx_delete(InvertedIndex invidx, Pointer student, bool delete_entry, Pointer *old_entry);

// Get the List of student at year
List invidx_students_at(InvertedIndex invidx, int year);

// Free all the memory
void invidx_destroy(InvertedIndex invidx);
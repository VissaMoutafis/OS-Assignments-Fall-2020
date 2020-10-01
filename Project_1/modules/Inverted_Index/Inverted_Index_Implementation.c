/*
**  Inverted Index Data Structure Implementation
*/

#include "InvertedIndex.h"

struct inverted_index {
    List *indexes;
    Compare compare;
    ItemDestructor itemDestructor;
    size_t item_count;
};

struct index_struct {
    int year;            // year of studies
    List students;           // students at the same study year 
};

// Utilities

Index create_index(int year, Compare compare, ItemDestructor destructor) {
    Index idx = malloc(sizeof(*idx));

    idx->year = year;
    idx->students = list_create(compare, destructor);

    return idx;
}
compare_index(Pointer y1, Pointer y2) {
    return ((Index)y1)->year - ((Index)y2)->year;
}

void delete_index(Pointer i) {
    Index x = (Index)i;
    list_destroy(&(x->students));
    free(i);
}
// Inverted Index D.S. Methods

// Constructor Function
InvertedIndex invidx_create( Compare compare, ItemDestructor itemDestructor) {
    InvertedIndex invidx = malloc(sizeof(*invidx));

    // Initialization
    invidx->compare = compare;
    invidx->itemDestructor = itemDestructor;
    invidx->indexes = list_create(compare_index, delete_index);
    invidx->item_count = 0;
    

    return invidx;
}

// Simple student insertion function
void invidx_insert(InvertedIndex invidx, Pointer student) {
    assert(invidx);
    assert(student);
    // dummy entry for search
    Index dummy = malloc(sizeof(*dummy));
    dummy->year = (CUR_YEAR - ((Student)student)->year_of_registration) + 1;

    // we get the list node that contains the Index as entry
    ListNode cur = list_find(invidx->indexes, dummy);
    if (cur) {
        // if such list exist
        // make clean that the entry is an Index
        Index entry = (Index)list_node_get_entry(invidx->indexes, cur);
        List std_list = entry->students;

        // Now we make sure that this student does not exist in the list
        if (!list_find(std_list, student))
            // if not then add the student
            list_insert(std_list, student, true);

    } else {
        // case that the list does not exist

        // create new index with a list 
        Index new_index = create_index(dummy->year, invidx->compare, invidx->compare); 
        
        // add the student to the index list
        list_insert(new_index->students, student, true);
        
        // add the new index to the inverted index struct
        list_insert(invidx->indexes, new_index, true);
    }
}


// Delete a student from the index
// if delete_entry == true then we free the student struct memory
// otherwise we will keep the record at the old_entry pointer
void invidx_delete(InvertedIndex invidx, Pointer student, bool delete_entry, Pointer *old_entry) {
    assert(invidx);
    assert(student);

    *old_entry = NULL;

    // dummy entry for search
    Index dummy = malloc(sizeof(*dummy));
    dummy->year = (CUR_YEAR - ((Student)student)->year_of_registration) + 1;

    // we get the list node that contains the Index as entry
    ListNode cur = list_find(invidx->indexes, dummy);
    if (cur) {
        // if such list exist
        // make clean that the entry is an Index
        Index entry = (Index)list_node_get_entry(invidx->indexes, cur);
        List std_list = entry->students;

        // If the node exists in the index list delete it
        if (list_find(std_list, student)) 
            list_delete(std_list, student, true, old_entry);
    }
}

void invidx_students_at(InvertedIndex invidx, int year) {
    // dummy entry for search
    Index dummy = malloc(sizeof(*dummy));
    dummy->year = year;

    // we get the list node that contains the Index as entry
    ListNode cur = list_find(invidx->indexes, dummy);
    
    if (cur) {
        // if such list exist
        // make clean that the entry is an Index
        Index entry = (Index)list_node_get_entry(invidx->indexes, cur);
        return entry->students;
    }
    
    // if we reached here then there is no node that contains an index for the given year
    return NULL;
}

void invidx_destroy(InvertedIndex invidx) {
    // we must destroy the list. The others are destroyed automatically due to the use
    // of item destructors
    list_destroy(&(invidx->indexes));

    // now free the whole struct
    free(invidx);
}
/*
**  Inverted Index Data Structure Implementation
*/

#include "InvertedIndex.h"

struct inverted_index {
    List *indexes;
    Compare index_compare;
    ItemDestructor index_itemDestructor;
    size_t item_count;
};

typedef struct index_struct {
    size_t year;            // year of studies
    AVL students;           // students at the same study year 
} *Index;

// Utilities

Index create_index(size_t year, Compare index_compare, ItemDestructor index_destructor) {
    Index idx = malloc(sizeof(*idx));

    idx->year = year;
    idx->students = list_create(index_compare, index_destructor);

    return idx;
}
//////////////////////////////////////////////////
compare_index(Pointer y1, Pointer y2) {
    return ((Index)y1)->year - ((Index)y2)->year;
}
void delete_index(Pointer i) {
    Index x = (Index)i;
    list_destroy(&(x->students));
    free(x);
}
// ////////////////////////////////////////////////////////
// Inverted Index D.S. Methods

// Constructor Function
InvertedIndex invidx_create(Compare index_compare, ItemDestructor index_itemDestructor, Compare compare, ItemDestructor itemDestructor) {
    InvertedIndex invidx = malloc(sizeof(*invidx));

    // Initialization
    invidx->index_compare = index_compare;
    invidx->index_itemDestructor = index_itemDestructor;
    invidx->indexes = list_create(compare, itemDestructor);
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
        AVL std_tree = entry->students;

        // Now we make sure that this student does not exist in the list
        if (!avl_find(std_tree, student))
            // if not then add the student
            avl_insert(std_tree, student);

    } else {
        // case that the list does not exist

        // create new index
        Index new_index = create_index(dummy->year, invidx->index_compare, invidx->index_compare); 
        
        // add the student to the index list
        avl_insert(new_index->students, student);
        
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
        AVL std_tree = entry->students;

        // If the node exists in the index list delete it
        if (avl_find(std_tree, student)) 
            avl_delete(std_tree, student, true, old_entry);
    }
}


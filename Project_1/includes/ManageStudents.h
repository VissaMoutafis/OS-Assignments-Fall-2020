#pragma once

#include "HT.h"
#include "List.h"
#include "ParsingUtils.h"
#include "TTY.h"
#include "InvertedIndex.h"
#include "StructManipulation.h"

#define DEFAULT_ENTRIES 500

typedef struct mngstd* ManageStudents;

// create the app struct
ManageStudents mngstd_create(Compare std_compare, ItemDestructor std_destructor, Hash_Func std_hash_func, char* in_filename);

// program run
void mngstd_run(ManageStudents manager, int expr_index, char* value);

// free the memory
void mngstd_destroy(ManageStudents manager);

// Global variable for exit functionality
bool is_end;
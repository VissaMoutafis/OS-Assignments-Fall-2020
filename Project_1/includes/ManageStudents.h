#pragma once

#include "HT.h"
#include "List.h"
#include "ParsingUtils.h"
#include "TTY.h"
#include "InvertedIndex.h"

#define DEFAULT_ENTRIES 500

typedef struct mngstd* ManageStudents;

// create the app struct
ManageStudents mngstd_create(Compare std_compare, ItemDestructor std_destructor, Hash_Func std_hash_func, char* in_filename);

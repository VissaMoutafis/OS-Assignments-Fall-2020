#pragma once
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

typedef void *Pointer; //We will use the Pointer notation for Item and/or Key type

typedef int (*Compare)(Pointer item1, Pointer item2); //Function to compare 2 items given by the user

typedef void (*ItemDestructor)(Pointer item); //Function to delete the items since the user allocates the memory for the items all alone

typedef void (*Visit)(Pointer item); //Function to help the user define the way the items are printed

typedef size_t (*Hash_Func)(Pointer entry); //Function to hash the entry to a specific integer

typedef struct {
    char *l;
    char *u;
} Range;

#define PRIME_ALGO_PATH "./src/modules/prime-finding-programs/"
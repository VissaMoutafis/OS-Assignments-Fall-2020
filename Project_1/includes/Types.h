#pragma once
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>


// THIS FILE CONTAINS SOME BASIC AND FUNDAMENTAL TYPEDEFS
// THAT HELP WITH THE OVERALL ABSTRACTION LEVELING OF THE PROJECT
int cur_year;
bool cnfg;

typedef void *Pointer; //We will use the Pointer notation for Item and/or Key type

typedef int (*Compare)(Pointer item1, Pointer item2); //Function to compare 2 items given by the user

typedef void (*ItemDestructor)(Pointer item); //Function to delete the items since the user allocates the memory for the items all alone

typedef void (*Visit)(Pointer item); //Function to help the user define the way the items are printed

typedef size_t (*Hash_Func)(Pointer entry); //Function to hash the entry to a specific integer


// The Student struct

typedef struct std {
    char *student_id;
    char *first_name;
    char *last_name;
    char *postal;
    int year_of_registration;
    float gpa;

} *Student;
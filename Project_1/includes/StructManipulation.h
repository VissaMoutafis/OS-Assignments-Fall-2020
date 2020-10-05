/*
** This header file declares the constructors, destructors, 
** comparison functions, hash functions and any other utilitie we use to 
** manipulate the user defined structs. 
*/

#pragma once

#include "Types.h"

typedef struct zip_code_count {
    char* postal_code;
    int count;
} *ZipCount;

typedef struct count_per_year {
    int year;
    int count;
}* CountPerYear;

// Function Declarations
Pointer create_std(char *student_id, char *last_name, char *first_name , char *postal, int year_of_rgstr, float gpa, bool deep_copy);
int student_compare(Pointer s1, Pointer s2);               // students comparison function
void student_destructor(Pointer s);                        // student destruction function
size_t student_hash(Pointer s);                            // student hashing function
void student_visit(Pointer s);                             // student printing function
int std_gpa_compare(Pointer s1, Pointer s2);               // student gpa comparison function
Pointer create_zip_count(char *zip, int count, bool deep_copy); // constructor
int zip_code_compare(Pointer zip1, Pointer zip2);          // zip code compare function
void zip_code_destructor(Pointer zip);                     // zip code struct destructor
void zip_code_print(Pointer zip);                          // zip code struct visit function
void destroy_count_year_pair(Pointer pair);                // pair destructor
void print_count_year_pair(Pointer pair);                  // pair visit function
int compare_count_year_pair(Pointer pair1, Pointer pair2); // pair compare function

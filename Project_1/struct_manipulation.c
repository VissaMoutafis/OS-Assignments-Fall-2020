#include "StructManipulation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Pointer create_std(char *student_id, char *last_name, char *first_name , char *postal, int year_of_rgstr, float gpa, bool deep_copy) {
    Student s = malloc(sizeof(*s));

    if (!deep_copy) {
        // shallow copy
        s->first_name = first_name;
        s->last_name = last_name;
        s->student_id = student_id;
        s->postal = postal;

    } else {
        // deep copy
        s->student_id = student_id ? calloc(strlen(student_id)+1, sizeof(char)) : NULL;
        if (student_id) strcpy(s->student_id, student_id);

        s->first_name = first_name ? calloc(strlen(first_name)+1, sizeof(char)) : NULL;
        if (first_name) strcpy(s->first_name, first_name);

        s->last_name = last_name ? calloc(strlen(last_name)+1, sizeof(char)) : NULL;
        if (last_name) strcpy(s->last_name, last_name);

        s->postal = postal ? calloc(strlen(postal)+1, sizeof(char)) : NULL;
        if (postal) strcpy(s->postal, postal);
    }

    // arithmetics are the same in deep and shallow copy
    s->gpa = gpa;
    s->year_of_registration = year_of_rgstr;

    return s;
}


int student_compare(Pointer s1, Pointer s2) {
    Student std1 = (Student)s1, std2 = (Student)s2;
    return strcmp(std1->student_id, std2->student_id);
}

int std_gpa_compare(Pointer s1, Pointer s2) {
    Student std1 = (Student)s1, std2 = (Student)s2;
    if (std1->gpa - std2->gpa > 0.0)
        return 1;
    else if(std1->gpa - std2->gpa < 0.0)
        return -1;
    
    return 0;
}
void student_destructor(Pointer s) {
    Student std = (Student)s;
    if (std->first_name) free(std->first_name);
    if (std->last_name) free(std->last_name);
    if(std->postal) free(std->postal);
    if(std->student_id) free(std->student_id);
    free(s);
}

size_t student_hash(Pointer s) {
    Student std = (Student)s;
    char *str = std->student_id;
    size_t hashcode = 23;
    size_t sauce = 0;
    for (size_t i = 0; str[i] != '\0'; ++i)
    {
        size_t l = (size_t)(str[i] - '0');
        sauce += l;
        hashcode *= sauce;
        hashcode += l;
    }

    return hashcode;
}

void student_visit(Pointer s) {
    Student std = (Student)s;
    printf("%s %s %s %s %d %.2f\n",
           std->student_id, std->first_name, 
           std->last_name, std->postal, 
           std->year_of_registration, std->gpa);
}

void print_student_id(Pointer s) {
    Student std = (Student)s;
    printf("%s ", std->student_id);
}


Pointer create_zip_count(char* zip, int count, bool deep_copy) {
    ZipCount z = malloc(sizeof(*z));
    z->count = count;
    if (deep_copy) {
        z->postal_code = calloc(strlen(zip)+1, sizeof(char));
        strcpy(z->postal_code, zip);
    } else {
        z->postal_code = zip;
    }

    return (Pointer)z;
}

int zip_code_compare(Pointer zip1, Pointer zip2) {  
    ZipCount z1 = (ZipCount)zip1;
    ZipCount z2 = (ZipCount)zip2;
    return strcmp(z1->postal_code, z2->postal_code);
}

void zip_code_destructor(Pointer zip) {
    // we dont need to destroy the zip->postal_code since it is shallow copied
    free(zip);
}
void zip_code_print(Pointer zip) {
    ZipCount z = (ZipCount)zip;
    printf("%s ", z->postal_code);
}

int compare_count_year_pair(Pointer pair1, Pointer pair2) {
    CountPerYear p1 = (CountPerYear) pair1;
    CountPerYear p2 = (CountPerYear) pair2;
    return p1->year - p2->year;
}

void destroy_count_year_pair(Pointer pair) {
    free(pair);
} 
 
void print_count_year_pair(Pointer pair) {
    CountPerYear p = (CountPerYear)pair;
    printf("{%d, %d} ",CUR_YEAR - p->year + 1, p->count);
}

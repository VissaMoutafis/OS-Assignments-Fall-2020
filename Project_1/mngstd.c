#include "ManageStudents.h"

struct mngstd {
    HT students;
    InvertedIndex year_of_study_idx;
    size_t student_count;
};

/// Utility Functions
Pointer create_std(char *student_id, char *last_name, char *first_name , char *postal, size_t year_of_rgstr, float gpa) {
    Student s = malloc(sizeof(*s));

    s->first_name = first_name;
    s->last_name = last_name;
    s->student_id = student_id;
    s->gpa = gpa;
    s->postal = postal;
    s->year_of_registration = year_of_rgstr;
}
void initialize_with(char* filename, ManageStudents mngstd) {
    // open the file
    FILE* fin = fopen(filename, "r");
    // erro checking for the file
    if (fin) {
        // count the lines
        int lines = fget_lines(filename);
        for (int i = 0; i < lines; i++) {
            // take the data string from the file and turn it to a data table
            char* data_str = make_str(&fin);
            int data_cols;
            char** data_table = parse_std_data(data_str, &data_table);

            // create the student instance and add it in the structs
            Student std = create_std(data_table[0], 
                                    data_table[1], 
                                    data_table[2], 
                                    data_table[3], 
                                    strtoul(data_table[4], NULL), 
                                    strtof(data_table[5], NULL));
            ht_insert(mngstd->students, std);
            invidx_insert(mngstd->year_of_study_idx, std);

            // free the memory of helper variables
            free(data_str);
            free(data_str);
        }

        // number of insert = number of students = lines of the file
        mngstd->student_count = lines;
    
    } else {
        // error message
        printf("Warning: Cannot Open File with path'%s'\n", filename);
    }
}


// Manage Student D.S.

// constructor
ManageStudents mngstd_create(Compare std_compare, ItemDestructor std_destructor, Hash_Func std_hash_func, char* in_filename) {
    // create the struct
    ManageStudents mngstd = malloc(sizeof(*mngstd));    
    
    size_t num_of_entries = DEFAULT_ENTRIES;
    
    // creation
    mngstd->student_count = 0;    
    mngstd->students = ht_create(std_compare, std_hash_func, std_destructor, num_of_entries);
    mngstd->year_of_study_idx = invidx_create(std_compare, NULL);

    // file initialization
    initialize_with(in_filename, mngstd);

    return mngstd;
}

// TODO
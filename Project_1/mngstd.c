#include "ManageStudents.h"

struct mngstd {
    HT students;
    InvertedIndex year_of_study_idx;
    size_t student_count;
};

/// Utility Functions
Pointer create_std(char *student_id, char *last_name, char *first_name , char *postal, int year_of_rgstr, float gpa) {
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
    FILE* fin = filename != NULL ? fopen(filename, "r") : NULL;
    // erro checking for the file
    if (fin) {
        // count the lines
        int lines = fget_lines(filename);
        for (int i = 0; i < lines; i++) {
            // take the data string from the file and turn it to a data table
            char* data_str = make_str(&fin);
            int data_cols;
            char** data_table = parse_line(data_str, &data_table, ",");

            // create the student instance and add it in the structs
            Student std = create_std(data_table[0], 
                                    data_table[1], 
                                    data_table[2], 
                                    data_table[3], 
                                    strtol(data_table[4], NULL, 10), 
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
int get_restistrants_at(InvertedIndex invidx, int year) {
    List student_list = invidx_students_at(invidx, year);
    return student_list != NULL ? list_len(student_list) : 0;
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

void mngstd_destroy(ManageStudents manager) {
    ht_destroy(manager->students);
    invidx_destroy(manager->year_of_study_idx);
    free(manager);
}



void mngstd_run(ManageStudents manager, int expr_index, char* value) {
    if (expr_index == 0) {
        // command: insert, value: student data
        char **data_table = parse_line(value, &data_table, " ");

        // create the student instance and add it in the structs
        Student std = create_std(data_table[0],
                                 data_table[1],
                                 data_table[2],
                                 data_table[3],
                                 strtol(data_table[4], NULL, 10),
                                 strtof(data_table[5], NULL));
        ht_insert(manager->students, std);
        invidx_insert(manager->year_of_study_idx, std);

    } else if (expr_index == 1) {
        // command: look-up in ht, value: stuednt_id
        Pointer s;
        Student dummy = create_std(value, "", "", "", 0, 0.0);
        if (ht_contains(manager->students, dummy, &s)) {
            Student std = (Student)s;
            printf("> Student Info :\n");
            student_visit(std);
        } else {
            printf("> The student with student id : '%d' does not exist in the records.\n", value);
        }
        student_destructor(dummy);
    
    } else if (expr_index == 2) {
        // command: delete, value: student id
        Student dummy = create_std(value, "", "", "", 0, 0.0);
        Pointer s;
        if (ht_contains(manager->students, dummy, &s)) {
            // delete it first from the index
            invidx_delete(manager->year_of_study_idx, dummy, true, &s);
            // now delete it from the hash table to delete it normally
            ht_delete(manager->students, dummy, true, &s);
        }

        student_destructor(dummy);
    
    } else if (expr_index == 3) {
        // command: number of restistrants, value: alpharethmetic for the year
        if (is_numeric(value)) {
            int year = strtol(value, NULL, 10);

            // made up function to query the number of registrants in a year
            int students = get_restistrants_at(manager->year_of_study_idx, year); // To be implemented
            printf("> The number of students at year of study %d, is %d", year, students);
        } else {
            printf("The year must be a numeric value.\n");
        }    

    } else if (expr_index == 4) {
        // command: top n-th students, value: n year
        int cols;
        char ** data = parse_line(value, &cols, " ");
        if (cols < 3) {
            // find the top n students some how
        } else {
            printf("Too many arguments.\n");
            help();
        }

    } else if (expr_index == 5) {

    } else if (expr_index == 6) {

    } else if (expr_index == 7) {

    } else if (expr_index == 8) {

    } else if (expr_index == 9) {
        // command : exit, value = NULL
        is_end = true;
        return;
    } else {
        help();
    }
}

int student_compare(Pointer s1, Pointer s2) {
    Student std1 = (Student)s1, std2 = (Student)s2;
    return strcmp(std1->student_id, std2->student_id);
}

void student_destructor(Pointer s) {
    Student std = (Student)s;
    free(std->first_name);
    free(std->last_name);
    free(std->postal);
    free(std->student_id);
    free(std);
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
    printf("\n--------------------------------------------\nStudent Id: %s,\
     Name: %s, Surname: %s, Postal Code: %s,\
     Year of Registration: %lu, GPA: %.2f\
     \n--------------------------------------------\n")
}

int main(int argc, char **argv) {
    // the main logic of the menu is the following:
    // 1. get the input
    // 2. parse it
    // 3. give it to the runelection struct to make something out of it

    char **arguments;
    int size;
    args_parser(argc, argv, &arguments, &size); // Something must be done for the configurations
    ManageStudents manager = mngstd_create(student_compare, student_destructor, student_hash, argv[0]);
    is_end = false;
    
    while (manager && !is_end) {
        char *expr = get_input();
        char **parsed_cmd = parse_expression(expr); ///THIS FUNCTION MUST BE IMPLEMENTED
        unsigned int expr_index;
        if (is_valid(parsed_cmd[0], &expr_index) == true)
            mngstd_run(manager, expr_index, parsed_cmd[1]);
        else
            help();

        free(expr);
        free(parsed_cmd[0]);
        free(parsed_cmd[1]);
        free(parsed_cmd);
    }

    mngstd_destroy(manager); // to be implemented
}
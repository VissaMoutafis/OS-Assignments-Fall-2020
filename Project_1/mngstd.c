#include "ManageStudents.h"

struct mngstd {
    HT students;
    InvertedIndex year_of_study_idx;
    size_t student_count;
};

// Function Declarations
int student_compare(Pointer s1, Pointer s2);    // students comparison function
void student_destructor(Pointer s);             // student destruction function
size_t student_hash(Pointer s);                 // student hashing function
void student_visit(Pointer s);                  // student printing function
int std_gpa_compare(Pointer s1, Pointer s2);    // student gpa comparison function

/// Utility Functions

Pointer create_std(char *student_id, char *last_name, char *first_name , char *postal, int year_of_rgstr, float gpa, bool deep_copy) {
    Student s = malloc(sizeof(*s));

    if (deep_copy == false) {
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

static void insert_student(ManageStudents manager, char **data_table) {
    // create the student instance and add it in the structs
    Student std = create_std(data_table[0],
                             data_table[1],
                             data_table[2],
                             data_table[3],
                             strtol(data_table[4], NULL, 10),
                             strtof(data_table[5], NULL), false);

    // Insert the student record in the structs
    ht_insert(manager->students, std);
    invidx_insert(manager->year_of_study_idx, std);
    free(data_table[4]);
    free(data_table[5]);
    printf("> Student '%s' inserted.\n", data_table[0]);
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
            char** data_table = parse_line(data_str, &data_cols, " ");          
            // insert it 
            insert_student(mngstd, data_table);

            // free the memory of helper variables
            free(data_str);
            free(data_table);
        }

        // number of insert = number of students = lines of the file
        mngstd->student_count = lines;
    
    } else {
        // error message
        printf("> Warning: Cannot Open File with path'%s'\n", filename);
    }

    fclose(fin);
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
        int cols;
        char **data_table = parse_line(value, &cols, " ");
        Student dummy = create_std(data_table[0], NULL, NULL, NULL, 0, 0.0, true);
        Pointer entry;
        if (!ht_contains(manager->students, dummy, &entry)){    
            // REMEMBER TO FIND A WAY TO FILL INCOMPLETE ENTRIES
            insert_student(manager, data_table);
        } else {
            printf("> Student '%s' already exists.\n", data_table[0]);
            
            for (int i = 0; i < cols; i++)
                free(data_table[i]);
        }
        
        student_destructor(dummy);
        free(data_table);
    } else if (expr_index == 1) {
        // command: look-up in ht, value: stuednt_id
        Pointer s;
        Student dummy = create_std(value, NULL, NULL, NULL, 0, 0.0, true);
        if (ht_contains(manager->students, dummy, &s)) {
            Student std = (Student)s;
            printf("> Student Info : ");
            student_visit(std);
            printf("\n");
        } else {
            printf("> Student '%s' does not exist.\n", value);
        }
        student_destructor(dummy);
    
    } else if (expr_index == 2) {
        // command: delete, value: student id
        Student dummy = create_std(value, NULL, NULL, NULL, 0, 0.0, true);
        Pointer s;
        if (ht_contains(manager->students, dummy, &s)) {
            // delete it first from the index:
            // a fix because in inverted index you look based on year
            dummy->year_of_registration = ((Student)s)->year_of_registration; 
            invidx_delete(manager->year_of_study_idx, dummy, true, &s);
            
            // now delete it from the hash table to delete it normally
            ht_delete(manager->students, dummy, true, &s);
            printf("> Student %s deleted.\n", value);
        } else {
            printf("> Student %s does not exist.\n", value);
        }

        student_destructor(dummy);
    
    } else if (expr_index == 3) {
        // command: number of restistrants, value: alpharethmetic for the year
        int students = 0;
        if (is_numeric(value)) {
            int year = strtol(value, NULL, 10);
            students = get_restistrants_at(manager->year_of_study_idx, year);
            // made up function to query the number of registrants in a year
            printf("> %d student(s) in year %s.\n", students, value);
        } else {
            printf("> No students enrolled in %s.\n", value);
        }    

    } else if (expr_index == 4) {
        // command: top n-th students, value: n year
        int cols;
        char ** data = parse_line(value, &cols, " ");

        if (cols == 2 && is_numeric(data[0]) && is_numeric(data[1])) {
            int n = strtol(data[0], NULL, 10), year = strtol(data[1], NULL, 10);
            
            List student_list = invidx_students_at(manager->year_of_study_idx, year); // returns the invidx entry so dont destroy it
            if (student_list && list_len(student_list) > 0) {
                List top_n_th = list_get_top_n(student_list, std_gpa_compare, n);
                
                // print the list
                list_print(top_n_th, student_visit);
                // deallocate the memory
                list_destroy(&top_n_th);
            } else {
                printf("No students enrolled in year.\n");
            }
        } else {
            printf("No students enrolled in year.\n");
        }
        for (int i = 0; i < cols; i++) free(data[i]);
        free(data);

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

    free(value);
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

int main(int argc, char **argv) {
    // the main logic of the menu is the following:
    // 1. get the input
    // 2. parse it
    // 3. give it to the runelection struct to make something out of it

    char **arguments;
    int size;
    args_parser(argc, argv, &arguments, &size); // Something must be done for the configurations
    
    // Initiaze the struct
    ManageStudents manager = mngstd_create(student_compare, student_destructor, student_hash, arguments[0]);
    // turn the is_end pointer to false
    is_end = false;

    while (manager && !is_end) {
        // Get the tty expression
        char *expr = get_input();
        // Parse the expression
        char **parsed_cmd = parse_expression(expr); // parse the given experssion (format:~$ command value)
        
        // make clean what we are talking about
        char *command = parsed_cmd[0];
        char *value = parsed_cmd[1];
        int expr_index;

        // check if the expression is right and 
        if (check_format(command, &expr_index) == true)
            mngstd_run(manager, expr_index, value);
            // printf("The program will run {command: %s, value: %s}\n", parsed_cmd[0], parsed_cmd[1]);
        else
            help();

        free(expr);
        free(parsed_cmd[0]);
        free(parsed_cmd[1]);
        free(parsed_cmd);
    }

    // de-allocate the memory
    mngstd_destroy(manager);

    for(int i = 0; i < size; ++i) 
        free(arguments[i]);

    free(arguments);
}
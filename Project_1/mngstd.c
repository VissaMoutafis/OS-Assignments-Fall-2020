#include "ManageStudents.h"

struct mngstd {
    HT students;                        // Hash table for saving the student instances
    InvertedIndex year_of_study_idx;    // Inverted Index structure for the student year indexes
    List zip_codes_count;               // List for keeping the student-count per zip code
    size_t student_count;               // How many student do I got
};
typedef struct zip_code_count {
    char* postal_code;
    int count;
} *ZipCount;

// Function Declarations
int student_compare(Pointer s1, Pointer s2);    // students comparison function
void student_destructor(Pointer s);             // student destruction function
size_t student_hash(Pointer s);                 // student hashing function
void student_visit(Pointer s);                  // student printing function
int std_gpa_compare(Pointer s1, Pointer s2);    // student gpa comparison function
int zip_code_compare(Pointer zip1, Pointer zip2);// zip code compare function
void zip_code_destructor(Pointer zip);           // zip code struct destructor
void zip_code_print(Pointer zip);               // zip code struct visit function

/// Utility Functions

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
    
    // now we must check if the zip code exists.
    // if it is then increase the counter, else insert a ZipCount instance in the list
    ZipCount dummy = create_zip_count(data_table[3], 0, false);
    ListNode n = list_find(manager->zip_codes_count, dummy);
    if (n != LIST_EOF) {
        // if it exists increase the counter
        ZipCount zip_counter = (ZipCount)list_node_get_entry(manager->zip_codes_count, n); // get the entry
        zip_counter->count += 1; // increase the counter
    } else {
        // the entry doesn't exist => insert it
        // create a shallow copy and mind that the destructor won't touch the postal code string during 
        // exit memory deallocation
        list_insert(manager->zip_codes_count, create_zip_count(data_table[3], 0, false), true);
    }

    // deallocate the memory that is no longer needed
    zip_code_destructor(dummy);
    free(data_table[4]);
    free(data_table[5]);

    // print confirmation message
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

static int get_registrants_at(InvertedIndex invidx, int year) {
    List student_list = invidx_students_at(invidx, year);
    return student_list != NULL ? list_len(student_list) : 0;
}
// utility compare
int zip_counts_compare(Pointer zip1, Pointer zip2) {
    return ((ZipCount)zip1)->count - ((ZipCount)zip2)->count;
}
// src list, dest list = sorted, according to compare function
static List get_sorted_list(List list, List dest_sorted, Compare compare) {
    assert(list);
    assert(dest_sorted);

    ListNode cur = list_get_head(list);
    
    // Insert everything into dest_sorted list
    while (cur != LIST_EOF) {
        Pointer entry = list_node_get_entry(list, cur);
        list_insert_sorted(dest_sorted, entry, compare); // insert based on the popularity
        cur = list_get_next(list, cur); // traverse to the next node
    }
}

// returns the rank-th postal code(s) in a list. If there aren'e any return NULL. 
static List get_rankth_zip(List zip_codes, int rank) {
    // NOTE: !Be careful! We play around using *delegating* pointer. It needs concern.
    List sorted_zips = list_create(zip_code_compare, NULL); // dont delete the ZipCount instances

    // get the sorted list
    get_sorted_list(zip_codes, sorted_zips, zip_counts_compare);

    // we will find the head and tail of the rank-th zips in the sorted list 
    List rank_th_zips = list_create(zip_code_compare, NULL); // we don't want to mess with the ZipCount instances neither create new ones (memory recyclement)
    ListNode head, tail;
    head = list_get_head(sorted_zips);
    tail = NULL; 

    while(--rank && head) {
        Pointer entry = list_node_get_entry(sorted_zips, head);
        while (head && zip_counts_compare(entry, list_node_get_entry(sorted_zips, head)) == 0) {
            // traverse through the same popularity zips
            head = list_get_next(sorted_zips, head); // get to the next entry
        }
    }
    
    // head points to the first zip code of the rank_th order
    tail = head;
    Pointer entry = list_node_get_entry(sorted_zips, head);
    while (tail && zip_counts_compare(entry, list_node_get_entry(sorted_zips, tail)) == 0) {
        // traverse through the same popularity zips
        tail = list_get_next(sorted_zips, tail); // get to the next entry
    }
    // now the tails points to the next node of last zip code of the rank_th order
    // All that is left is to add all those zip code to the final list 
    while(head != tail) {
        // while we traverse in the set range
        Pointer entry = list_node_get_entry(sorted_zips, head);
        list_insert(rank_th_zips, entry, true); // insert the ZipCount instance
        head = list_get_next(sorted_zips, head); // continue to the next node
    }
    
    list_destroy(&sorted_zips); // de allocate the list

    if (list_len(rank_th_zips) == 0) {
        list_destroy(&rank_th_zips); // if the list is empty return NULL
        rank_th_zips = NULL;
    }

    return rank_th_zips;
}

static float avg_gpa(List std_list) {
    ListNode n = list_get_head(std_list);
    float sum = 0;
    while(n) {
        Pointer e = (Student)list_node_get_entry(std_list,n);
        sum += ((Student)e)->gpa;
        n = list_get_next(std_list, n);
    }

    return (float) (sum/((float)list_len(std_list)));
}

// returns min gpa student list, otherwise returns NULL
static List min_gpa_students(List std_list) {
    // Note: Be carefull to not de allocate the memory of the student structs
    // we will use it as it is to save some space and we will isolate this process in the function so the user
    // won't have to know where the pointers point to.
    
    if (!std_list) return NULL;
    
    List sorted = list_create(student_compare, NULL); // NULL destructor to clean only the list nodes 
    get_sorted_list(std_list, sorted, std_gpa_compare); // get the sorted version of the student list
    List min_students = list_create(student_compare, NULL);

    ListNode tail = list_get_tail(sorted); // the tail of the sorted list is the min node
    
    if (tail) {
        ListNode head = tail;
        Pointer min_val = list_node_get_entry(sorted, tail);

        // traverse back to the very first node in the list order that has the same gpa as the min_gpa node
        while(head && std_gpa_compare(min_val, list_node_get_entry(sorted, head)) == 0) {
            head = list_get_prev(sorted, head);
        }
        // at this point head->next points to the first student of the min list
        head = list_get_next(sorted, head);

        while (head) {
            Pointer student = list_node_get_entry(sorted, head);
            list_insert(min_students, student, true);
            head = list_get_next(sorted, head);
        }
    
    } else {
        list_destroy(&min_students);
        min_students = NULL;
    }

    list_destroy(&sorted);

    return min_students;
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
    mngstd->zip_codes_count = list_create(zip_code_compare, zip_code_destructor);
    // file initialization
    initialize_with(in_filename, mngstd);

    return mngstd;
}

void mngstd_destroy(ManageStudents manager) {
    ht_destroy(manager->students);
    invidx_destroy(manager->year_of_study_idx);
    list_destroy(&manager->zip_codes_count);
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
            // Before actual deletion we need to decrease the postal code count
            ZipCount zip_dummy = create_zip_count(((Student)s)->postal, 0, false);
            ListNode zip_n = list_find(manager->zip_codes_count, zip_dummy);
            ZipCount entry = (ZipCount)list_node_get_entry(manager->zip_codes_count, zip_n);
            entry->count -= 1;

            // Now we are ready: delete it first from the index:
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
        //NEEDS FIXING IS WRONG
        // command: number of registrants, value: alpharethmetic for the year
        int students = 0;
        if (is_numeric(value)) {
            int year = strtol(value, NULL, 10);
            students = get_registrants_at(manager->year_of_study_idx, year);
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
                printf("> No students enrolled in %s.\n", data[1]);
            }
            for (int i = 0; i < cols; i++)
                free(data[i]);
            free(data);
        } else {
            printf("> No students enrolled in %s.\n", data[1]);
        }
        

    } else if (expr_index == 5) {
        // command: avg, value: year
        if (is_numeric(value)) {
            int year = strtol(value, NULL, 10);
            // get the list check if empty
            List std_list = invidx_students_at(manager->year_of_study_idx, year);
            if (std_list && list_len(std_list) > 0) {
                printf("> Avg GPA for %s : %.2f\n", value, avg_gpa(std_list));
            } else {
                printf("> No students enrolled in %s\n", value);
            }
        } else {
            printf("> No students enrolled in %s\n", value);
        }
    } else if (expr_index == 6) {
        // command: min, value: year
        if (is_numeric(value)) {
            int year = strtol(value, NULL, 10);
            // get the list check if empty
            List std_list = invidx_students_at(manager->year_of_study_idx, year);
            if (std_list && list_len(std_list) > 0) {
                List min_gpa_std_list = min_gpa_students(std_list); // return the list of the students with the minimum gpa
                printf("> ");
                list_print(min_gpa_std_list, student_visit);
                printf("\n");
                list_destroy(&min_gpa_std_list);
            } else {
                printf("> No students enrolled in %s\n", value);
            }
        } else {
            printf("> No students enrolled in %s\n", value);
        }
    } else if (expr_index == 7) {

    } else if (expr_index == 8) {
        // command: postal, value: rank
        // we must find the rank-th most popular zip code 
        // one approach woul be to keep a list of all post codes and when we insert a student
        // we will update the according zip code or add it if it does not exist in the list.
        if (is_numeric(value)) {
            int rank = strtol(value, NULL, 10); // get the numeric value straight
            List zip_codes = get_rankth_zip(manager->zip_codes_count, rank);
            if (zip_codes != NULL) {
                printf("> ");
                list_print(zip_codes, zip_code_print);
                printf(" zip code(s) is/are rank : %s most popular.\n", value);
                list_destroy(&zip_codes);
            } else {
                printf("> No students are enrolled.\n");
            }
        } else {
            help();
        } 
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
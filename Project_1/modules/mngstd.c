#include "ManageStudents.h"

struct mngstd {
    HT students;                        // Hash table for saving the student instances
    InvertedIndex year_of_study_idx;    // Inverted Index structure for the student year indexes
    List zip_codes_count;               // List for keeping the student-count per zip code
    size_t student_count;               // How many student do I got
};

const char *error_expr[9] = {
    "> Student '%s' already exists.\n",
    "> Student '%s' does not exist.\n",
    "> Student %s does not exist.\n",
    "> No students enrolled in %s.\n",
    "> No students enrolled in %s.\n",
    "> No students enrolled in %s\n",
    "> No students enrolled in %s\n",
    "> No students are enrolled.\n",
    "> No students are enrolled.\n"};

/// Utility Functions   

void print_manager_error(int expr_index, char* value) {
    printf(error_expr[expr_index], value);
}

void insert_student(ManageStudents manager, char **data_table) {
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
        int lines = mngstd->student_count;
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

        fclose(fin);
    } else {
        // error message
        printf("> Warning: Cannot Open File with path'%s'\n", filename);
        is_end = true;
    }

    
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
static void get_sorted_list(List list, List dest_sorted, Compare compare) {
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


// simple function that returns all the years and their student counts from the inverted index
static List get_students_per_year(InvertedIndex index) {
    if (!index) return NULL;

    List std_per_year = invidx_to_list(index);
    List pairs = list_create(compare_count_year_pair, destroy_count_year_pair);
    // we must now traverse the list and get 2 things: the year and the student list length
    ListNode cur = list_get_head(std_per_year);

    while (cur) {
        Index index = (Index)list_node_get_entry(std_per_year, cur);
        int year = index_get_year(index);
        int count = list_len(index_get_list(index)); 
        CountPerYear pair = create_count_year_pair(year, count, true);
        list_insert(pairs, pair, true);
        cur = list_get_next(std_per_year, cur);
    }

    return pairs;
}

// Manage Student D.S.

// constructor
ManageStudents mngstd_create(Compare std_compare, ItemDestructor std_destructor, Hash_Func std_hash_func, char* in_filename) {
    // create the struct

    ManageStudents mngstd = malloc(sizeof(*mngstd));

    size_t num_of_entries = in_filename != NULL ? fget_lines(in_filename) : 0; // set the number of entries

    // creation
    mngstd->student_count = num_of_entries;
    mngstd->students = ht_create(std_compare, std_hash_func, NULL, num_of_entries ? num_of_entries : DEFAULT_ENTRIES);
    mngstd->year_of_study_idx = invidx_create(std_compare, std_destructor);
    mngstd->zip_codes_count = list_create(zip_code_compare, zip_code_destructor);
    // file initialization
    if (in_filename) 
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
    int cols;
    Pointer s;
    Student dummy;
    char **data;
    
    switch(expr_index) {
        case 0:
            // command: insert, value: student data
            if (!value) {help(); return;}
            
            data = parse_line(value, &cols, " ");

            if (cols == STUDENT_ATR_NUM) {
                dummy = create_std(data[0], NULL, NULL, NULL, 0, 0.0, true);
                Pointer entry;
                if (!ht_contains(manager->students, dummy, &entry)){    
                    insert_student(manager, data);
                } else {
                    print_manager_error(0, data[0]);
                    for (int i = 0; i < cols; i++)
                        free(data[i]);
                }
                student_destructor(dummy);
            } else {
                printf("> False number of Student attributes.\n");
                help();
                for (int i = 0; i < cols; i++) if (data[i]) free(data[i]);
            }
            
            free(data);
        break;

        case 1:
            // command: look-up in ht, value: student_id
            if (!value) {help(); return;}
            
            dummy = create_std(value, NULL, NULL, NULL, 0, 0.0, true);
            if (ht_contains(manager->students, dummy, &s)) {
                Student std = (Student)s;
                printf("> Student Info : ");
                student_visit(std);
                printf("\n");
            } else {
                print_manager_error(1, value);
            }
            student_destructor(dummy);
        
        break;

        case 2:
            // command: delete, value: student id
            if (!value) {help(); return;}
            
            dummy = create_std(value, NULL, NULL, NULL, 0, 0.0, true);
            if (ht_contains(manager->students, dummy, &s)) {
                // Before actual deletion we need to decrease the postal code count
                ZipCount zip_dummy = create_zip_count(((Student)s)->postal, 0, false);
                ListNode zip_n = list_find(manager->zip_codes_count, zip_dummy);
                ZipCount entry = (ZipCount)list_node_get_entry(manager->zip_codes_count, zip_n);
                free(zip_dummy);
                entry->count -= 1;
                
                // now delete firstly from the hash table to delete it normally
                ht_delete(manager->students, dummy, true, &s);

                // Now we are ready: delete it from the index:
                
                // a fix because in inverted index you search based on year
                dummy->year_of_registration = ((Student)s)->year_of_registration;
                invidx_delete(manager->year_of_study_idx, dummy, true, &s);

                printf("> Student %s deleted.\n", value);
            } else {
                print_manager_error(2, value);
            }

            student_destructor(dummy);
        
        break;

        case 3:
            // command: number of registrants, value: alpharethmetic for the year
            if (!value) {help(); return;}
            
            int students = 0;
            if (is_numeric(value)) {
                int year = strtol(value, NULL, 10);
                students = get_registrants_at(manager->year_of_study_idx, year);
                // made up function to query the number of registrants in a year
                if (students)
                    printf("> %d student(s) in year %s.\n", students, value);
                else
                    print_manager_error(3, value);
            } else {
                print_manager_error(3, value);
            }    

        break;

        case 4:
            // command: top n-th students, value: n year
            if (!value) {help(); return;}
            
            data = parse_line(value, &cols, " ");

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
                    print_manager_error(4, data[1]);
                }
            } else {
                help();
            }
            
            for (int i = 0; i < cols; i++)
                free(data[i]);
            free(data);

        break;

        case 5:
            // command: avg, value: year
            if (!value) {help(); return;}
            
            if (is_numeric(value)) {
                int year = strtol(value, NULL, 10);
                // get the list check if empty
                List std_list = invidx_students_at(manager->year_of_study_idx, year);
                if (std_list && list_len(std_list) > 0) {
                    printf("> Avg GPA for %s : %.2f\n", value, avg_gpa(std_list));
                } else {
                    print_manager_error(5, value);
                }
            } else {
                print_manager_error(5, value);
                help();
            }
        break;

        case 6:
            // command: min, value: year
            if (!value) {help(); return;}
            
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
                    print_manager_error(6, value);
                }
            } else {
                print_manager_error(6, value);
            }

        break;

        case 7:
            // command: count (students per year), value = (null)
            if (!value) {
                List index_list = invidx_to_list(manager->year_of_study_idx);
                if (index_list && list_len(index_list) > 0) {
                    List students_per_year = get_students_per_year(manager->year_of_study_idx);
                    printf("> ");
                    list_print(students_per_year, print_count_year_pair);
                    printf("\n");
                    list_destroy(&students_per_year);
                } else {
                    print_manager_error(7, NULL);
                }
            } else {
                help();
            }
        break;

        case 8:
            // command: postal, value: rank
            // we must find the rank-th most popular zip code 
            // one approach woul be to keep a list of all post codes and when we insert a student
            // we will update the according zip code or add it if it does not exist in the list.
            if (value && is_numeric(value)) {
                int rank = strtol(value, NULL, 10); // get the numeric value straight
                List zip_codes = get_rankth_zip(manager->zip_codes_count, rank);
                if (zip_codes != NULL) {
                    printf("> ");
                    list_print(zip_codes, zip_code_print);
                    printf(" zip code(s) is/are rank : %s most popular.\n", value);
                    list_destroy(&zip_codes);
                } else {
                    print_manager_error(8, NULL);
                }
            } else {
                help();
            }
        break;

        case 9:
            // command : exit, value = NULL
            is_end = true;
            printf("Exiting...\n");
            return;
        default:
            help();
    }
}

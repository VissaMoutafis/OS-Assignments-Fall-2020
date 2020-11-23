// Chef Code Base by Vissarion Moutafis

#include "Chef.h"
#include "ParsingUtils.h"

// // create the worker processes 
// static pid_t hire_salad_workers (int number_of_workers) {
//     pid_t pid = 0;
//     // create the children processes (salad workers) 
//     for (int i = 0; i < number_of_workers) {
//         pid = fork();
//         if (pid)
//             break;
//     }

//     // return the pid's
//     return pid;
// }

// // get the workers to work
// static void start_shift(int lowest_salad_id, int highest_salad_id, Ingredients available_ingredient) {
    
// }

// static void close_store(shmid_ds shm_table[], int shm_table_size, sem_t sem_table[], int sem_table_size) {
//     printf("\nClosing the store...\n");
//     // free the shared segment
//     free_shm(shm_table, shm_table_size);
//     // close the semaphores
//     close_sem(sem_table, sem_table_size);
//     wait_children();

//     printf("Bye!\n");
// }

// static void take_a_break(int mantime) {
//     // basic sleep
//     sleep(mantime);
// }

// static void chef_behaviour(void) {
//     // TODO : pick 2 ingredients, wait for a little. Repeat
// }

static void print_usage(void) {
    fprintf(stderr, "\n   Usage: ~$ ./chef -n [Number Of Salads] -m [mantime]\n");
}

static bool parse_args(int argc, char* argv[], char* proper_args[], int proper_args_size, char* num_args[], int num_args_size, char ***parsed) {
    // we assume the user allocated the proper memory for the parsed array
    
    if (check_args(argc, argv, proper_args, proper_args_size, num_args, num_args_size) == false) {
        print_usage();
        return false;
    }
    *parsed = calloc(proper_args_size, sizeof(char*));

    for (int i = 1; i < argc; i += 2) {
        int id = find_arg_index(proper_args, proper_args_size, argv[i]);
        assert(id > -1);
        // assign the value
        
        (*parsed)[id] = argv[i+1];
    }

    return true;
}

// called as "./chef -n [numOfSlds] -m mantime"
int main(int argc, char* argv[]) {
    char *proper_args[] = {"-n", "-m"};
    char **parsed = NULL;    

    bool ret_val = parse_args(argc, argv, proper_args, 2, proper_args, 2, &parsed);
    
    if (ret_val == false)
        exit(1);
    
    printf("All good\n");
    for (int i = 0; i < 2; i++) printf("%s -> %s\n", proper_args[i], parsed[i]);

    free(parsed);
    // create_shared_memory
    // create_sem
    // pid_t pid = hire_salad_workers();

    // if (!pid) 
    //     chef_behaviour()
    // else 
    //     start_shift()

    exit(0);
}
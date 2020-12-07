// Chef Code Base by Vissarion Moutafis

#include "Chef.h"
#include "ParsingUtils.h"

char* available_resources[] = {TOMATO, ONION, PEPPER}; // salad-maker main resource array
int init_salads=0; // needed for logging
sem_t *mutex, *log_mutex;      // global mutex semaphore
FILE* logfile, *common_log; /// global personal and public log file streams


// Provide the caller with the commmand you should call the workers
static void wait_for_workers(Order order, int num_of_workers, int num_of_resources) {
    printf("Ready to call the salad-maker programs. \n \
    Call them with the following command line instructions:\n");
    for (int i = 0; i < num_of_workers; ++i) {
        int t1, t2;
        t1 = get_int_in(0, MAX_PREP_TIME-1);
        t2 = get_int_in(t1, MAX_PREP_TIME);
        printf("./salad-maker -t1 %d -t2 %d -s %d -i %s &", t1, t2, order.shm_id, available_resources[i%num_of_resources]);
        if (i == num_of_workers-1)
            printf("\n");
    }
}

static void choose_ingr(char *a, char *b, int ingr_id) {
    if (ingr_id == 0) {
        strcpy(a, ONION);
        strcpy(b, PEPPER);
    } else if (ingr_id == 1) {
        strcpy(a, TOMATO);
        strcpy(b, PEPPER);
    } else {
        strcpy(a, TOMATO);
        strcpy(b, ONION);
    }
} 

// provide ingredients (signal a random semaphore that is never equal to prev)
static int provide_ingredients(Ingredients ingr[], int size, int prev) {
    int ingr_id;
    do {
        ingr_id = rand() % size;
    } while (prev == ingr_id);

    /// print the log
    char a[40],b[40];
    choose_ingr(a, b, ingr_id);
    char msg[150];
    sprintf(msg, "Selecting ingredients '%s', '%s'", a, b);
    print_log(log_code_select_ingr, logfile, "Chef", msg, NULL);
    print_log(log_code_select_ingr, common_log, "Chef", msg, log_mutex);
    // provide the salad maker with 2 ingredients
    sem_V(ingr[ingr_id]);
    
    // print the proper log messages
    sprintf(msg, "Notify Saladmaker%d", ingr_id+1);
    print_log(log_code_notify, logfile, "Chef", msg, NULL);
    print_log(log_code_notify, common_log, "Chef", msg, log_mutex);
    return ingr_id;
}

// take a break (sleep for mantime seconds)
static void take_a_break(int mantime) {
    // basic sleep
    char msg[150];
    sprintf(msg, "Mantime for %d sec", mantime);
    print_log(log_code_sleep, logfile, "Chef", msg, NULL);
    print_log(log_code_sleep, common_log, "Chef", msg, log_mutex);

    sleep(mantime);
}

// check (atomicaly) if the order is completed (shared memory: the salad count is zero)
static bool check_done(Order order) {
    // we need to check everything atomicaly
    sem_P(mutex);
    int n = order.shmem->num_of_salads;
    sem_V(mutex);

    return n <= 0;
}

static bool check_workers_done(Order *order) {
    // we need to check everything atomicaly
    sem_P(mutex);
    int n = order->shmem->num_of_finished;
    sem_V(mutex);

    return n == 3;
}

// function to print the final statistics for the salad makers 
static void print_result_statistics(Order order) {
    char msg[BUFSIZ], temp_msg[BUFSIZ];
    memset(msg, 0, BUFSIZ);
    memset(temp_msg, 0, BUFSIZ);
    int salads_per_saladmaker[3];
    int concurrent_size = 0;
    memcpy(salads_per_saladmaker, order.shmem->salads_per_saladmaker, sizeof(salads_per_saladmaker));
    int interval_counters[3] = {0, 0, 0};
    char *usr[3] = {"Saladmaker1", "Saladmaker2", "Saladmaker3"};
    MyTimeInterval ** intervals = get_time_intervals_from_log(LOG_PATH, log_code_cook_start, log_code_cook_end, usr, 3, interval_counters);
    MyTimeInterval * concurrent_work_intervals = find_concurrent_intervals(intervals, 3, interval_counters, &concurrent_size);
    printf("Total #Salads %d/%d\nSalads per salad maker {%d, %d, %d}\nConcurrent work time intervals: ",
            init_salads - order.shmem->num_of_salads, 
            init_salads,
            salads_per_saladmaker[0], 
            salads_per_saladmaker[1],
            salads_per_saladmaker[2]);
    strcat(msg, temp_msg);

    for (int i = 0; i < concurrent_size; i ++) {
        char start_buf[12], end_buf[12];
        memset(start_buf, 0, 12);
        memset(end_buf, 0, 12);
        MyTime_time_to_str(&concurrent_work_intervals[i].start, start_buf, 12);
        MyTime_time_to_str(&concurrent_work_intervals[i].end, end_buf, 12);
        printf("\n[%s - %s] ", start_buf, end_buf);
        strcat(msg, temp_msg);
    }
    puts("\n");

        // for (int i = 0; i < 3; i ++) {
        //     printf("Time intervals for %s:\n", usr[i]);
        //     for (int j = 0; j < interval_counters[i]; j++) {
        //         printf("[%d:%d:%d:%d - %d:%d:%d:%d] ",
        //         intervals[i][j].start.hour,
        //                intervals[i][j].start.min, intervals[i][j].start.sec,
        //                intervals[i][j].start.millisec,
        //                intervals[i][j].end.hour, intervals[i][j].end.min,
        //                intervals[i][j].end.sec,
        //                intervals[i][j].end.millisec);
        //     }
        //     printf("\n");
        // }

    for (int i = 0; i < 3; i ++)
            free(intervals[i]);

    free(intervals);
    free(concurrent_work_intervals);
}

// main loop for the chef behaviour
static void chef_behaviour(Order order, Ingredients ingr[], int ingr_size, sem_t* table, int mantime) {
    int prev_ingr_id = -1;
    while (!check_done(order)){
        // while there are salads to be cooked

        // try to use the table (check if the table is empty)
        printf("Checking if table is free.\n");
        sem_P(table);
        printf("Table is free\n"); //REMEBER TO REMOVE

        // try to provide an ingredient and get the ingredient id for the next iteration
        prev_ingr_id = provide_ingredients(ingr, ingr_size, prev_ingr_id);
        
        // relax for mantime seconds (process turn to idle)
        take_a_break(mantime);
    }
}

// function to dettach and destroy relative shared memory segments (shmids and semaphores)
static void close_store(char** sem_names, sem_t** sem, int sem_size, ShmPair* shm_table, int shm_table_size) {    
    // shared memory segments destruction
    for (int i = 0; i < sem_size; i++) 
        sem_clear(sem_names[i], sem[i]);
    
    // semaphore destruction
    for (int i = 0; i < shm_table_size; i++)
        shm_destroy(shm_table[i].shm_id);
}

// function to create the order (the shared memory part)
static int create_order(Order *order, int number_of_salads) {
    // create the shared memory (get the id)
    order->shm_id = shm_create(sizeof(SharedMem));
    if (order->shm_id < 0) {
        perror("creating order");
        return ORDER_FAILURE_CRT;
    }
    // some logging for later use
    printf("Shared Memmory ID: %d\n", order->shm_id);
    
    // if everything ok, then attach the process to the shared memory part
    order->shmem = (SharedMem*)shm_attach(order->shm_id);
    // initialize the shared segment
    order->shmem->num_of_salads = number_of_salads;
    // initialize the salads/salad maker, counters
    for (int i = 0; i < 3; i++) order->shmem->salads_per_saladmaker[i] = 0;
    
    // initialize the global variable for future logging
    init_salads = number_of_salads;

    // return success
    return ORDER_SUCCESS;
}

// classic print usage prompt
static void print_usage(void) {
    fprintf(stderr, "\n   Usage: ~$ ./chef -n [Number Of Salads] -m [mantime]\n");
}

// function to check and parse the arguments (checked and working)
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
    // initialize the rand seed
    srand((unsigned int) time(NULL));

    // check and parse the arguments
    char *proper_args[] = {"-n", "-m"};
    char **parsed = NULL;    
    bool ret_val = parse_args(argc, argv, proper_args, 2, proper_args, 2, &parsed);
    if (ret_val == false)
        exit(1);
    
    // transform the arguments to int
    int number_of_salads = atoi(parsed[0]);
    int mantime = atoi(parsed[1]);

    // open up both personal and public (close public and reopen it for append only use)
    logfile = fopen("./logs/chef", "w");
    common_log = fopen(LOG_PATH, "w");
    fclose(common_log);
    common_log = fopen(LOG_PATH, "a");
    
    // create the shared memory part
    Order order;
    if (create_order(&order, number_of_salads) == ORDER_FAILURE_CRT) {
        print_error("Salad order was not created.");
    }

    // create the semaphores
    Ingredients tomato, onion, pepper;
    tomato = sem_create(TOMATO, 0);
    onion = sem_create(ONION, 0);
    pepper = sem_create(PEPPER, 0);
    Ingredients ingr[] = {tomato, onion, pepper};
    // create a mutex so that every action is done atomicaly
    mutex = sem_create(MUTEX, 1);
    // create a log mutex so that the process writes to the public log atomically
    log_mutex = sem_create(LOG_MUTEX, 1);
    // the semaphore that will signal the end of all workers (wait on it before free everything)
    // the semaphore that will signal when the salad maker took the ingredients, chef provided him with
    sem_t *table = sem_create(WORKING_TABLE, 1);

    // print out the workers calling lines and wait for them (user initiated)
    wait_for_workers(order, 3, 3);
    // go to the chef behaviour (main loop and final logging at the end)
    chef_behaviour(order, ingr, 3, table, mantime);

    ShmPair shm_table[] = {order};


    printf("Trying to clear (waiting for workers to finish)\n");
    // wait for every body to finish
    while (!check_workers_done(&order));

    // close the store (dettach everything and release them properly)
    sem_t *sems[] = {tomato, onion, pepper, mutex, log_mutex, table};
    char * names[] = {TOMATO, ONION, PEPPER, MUTEX, LOG_MUTEX, WORKING_TABLE};
    close_store(names, sems, 6, shm_table, 1);
    printf("Done clearing\n");
    // print final results
    print_result_statistics(order);
    
    fclose(logfile);
    fclose(common_log);
    free(parsed);
    exit(0);
}
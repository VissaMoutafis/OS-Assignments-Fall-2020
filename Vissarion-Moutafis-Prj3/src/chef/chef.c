// Chef Code Base by Vissarion Moutafis

#include "Chef.h"
#include "ParsingUtils.h"

char* available_resources[] = {TOMATO, ONION, PEPPER};
int init_salads=0;
sem_t *mutex;
FILE* logfile;

static void wait_for_workers(Order order, int num_of_workers, int num_of_resources) {
    printf("Ready to call the salad-maker programs. \
    Call them with the following command line instructions:\n");
    for (int i = 0; i < num_of_workers; ++i) {
        int t1, t2;
        t1 = get_int_in(0, MAX_PREP_TIME-1);
        t2 = get_int_in(t1, MAX_PREP_TIME);
        printf("./salad-maker -t1 %d -t2 %d -s %d -i %s", t1, t2, order.shm_id, available_resources[i%num_of_resources]);
        if (i != num_of_workers)
            printf(" & ");
        else 
            printf("\n");
    }
    printf("\nPress any key, when ready: ");
    getchar();
}

static int provide_ingredients(Ingredients ingr[], int size, int prev) {
    int ingr_id;
    do {
        ingr_id = rand() % size;
    } while (prev == ingr_id);

    // provide the salad maker with 2 ingredients
    sem_V(ingr[ingr_id]);
    char msg[150];
    sprintf(msg, "Signal to salad-maker with main resource '%s', you're ready to cook\n", available_resources[ingr_id]);
    print_log(logfile, msg);
    
    return ingr_id;
}

static void take_a_break(int mantime) {
    // basic sleep
    sleep(mantime);
}

static bool check_done(Order order) {
    // we need to check everything atomicaly
    sem_P(mutex);
    int n = order.shmem->num_of_salads;
    sem_V(mutex);

    return n <= 0;
}

static void check_concurrent_workers(sem_t* tables, struct tm** start) {
    struct tm* time = get_time();
    if (sem_P_nonblock(tables) == -1) {
        // they work concurrently
        // check if its start time
        if (!(*start))
            *start = time;
    } else if ((*start) != NULL) {
        // they don't work concurrently (but they did before (start was not null))
        // concurrency_handler(*start, time);
        *start = NULL;
    } else {
        // they haven't been working concurrently as far as we know
        *start = NULL;
    }
}

static void print_result_statistics(Order order) {
    char msg[300];
    int salads_per_saladmaker[3];
    memcpy(salads_per_saladmaker, order.shmem->salads_per_saladmaker, sizeof(salads_per_saladmaker));
    sprintf(msg, "Results: Salads Done %d/%d,  Salads per salad maker {%d, %d, %d}",
            init_salads - order.shmem->num_of_salads, init_salads,
            salads_per_saladmaker[0], salads_per_saladmaker[1],
            salads_per_saladmaker[2]);
    print_log(logfile, msg);
}

static void chef_behaviour(Order order, Ingredients ingr[], int ingr_size, sem_t* tables, int mantime) {
    // print beggining log
    char msg[150];
    sprintf(msg, "Chef begins distributing ingredients to the salad makers at %s", get_time_str());
    print_log(logfile, msg);

    int prev_ingr_id = -1;
    struct tm* concurrent_start = NULL;

    while (!check_done(order)){
        prev_ingr_id = provide_ingredients(ingr, ingr_size, prev_ingr_id);
        // check if they worked together
        check_concurrent_workers(tables, &concurrent_start);
        // relax for mantime seconds (process turn to idle)
        take_a_break(mantime);
        // check if they worked together
        check_concurrent_workers(tables, &concurrent_start);
    }
    // print ending log and results
    sprintf(msg,"Chef stopped ingredients distribution at %s", get_time_str());
    print_log(logfile, msg);
    print_result_statistics(order);
}

// function to dettach and destroy relative shared memory segments (shmids and semaphores)
static void close_store(sem_t* workers_done, char** ingr_names, Ingredients* ingr, int ingr_size, ShmPair* shm_table, int shm_table_size) {
    // wait for workers to end
    sem_P(workers_done);
    sem_clear(SALAD_WORKER, workers_done);
    // shm segs destruction
    for (int i = 0; i < ingr_size; i++) 
        sem_clear(ingr_names[i], ingr[i]);
    
    // semaphore destruction
    for (int i = 0; i < shm_table_size; i++)
        shm_destroy(shm_table[i].shm_id);
}


static int create_order(Order *order, int number_of_salads) {
    // create the shared memory
    order->shm_id = shm_create(sizeof(SharedMem));
    if (order->shm_id < 0) {
        perror("creating order");
        return ORDER_FAILURE_CRT;
    }
    fprintf(logfile, "Shared Memmory ID: %d\n", order->shm_id);
    // if everything ok, then attach the process to the shared memory part
    order->shmem = (SharedMem*)shm_attach(order->shm_id);
    // initialize the shared segment
    order->shmem->num_of_salads = number_of_salads;
    for (int i = 0; i < 3; i++) order->shmem->salads_per_saladmaker[i] = 0;
    init_salads = number_of_salads;
    return ORDER_SUCCESS;
}

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
    system("clear");
    
    srand((unsigned int) time(NULL));

    char *proper_args[] = {"-n", "-m"};
    char **parsed = NULL;    

    bool ret_val = parse_args(argc, argv, proper_args, 2, proper_args, 2, &parsed);
    
    if (ret_val == false)
        exit(1);
    
    
    // transform the arguments to int
    int number_of_salads = atoi(parsed[0]);
    int mantime = atoi(parsed[1]);
    logfile = fopen("./logs/chef", "w");
    if (logfile == NULL) {
        perror("fopen");
    }
    free(parsed);
    
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
    sem_t *children_done = sem_create(SALAD_WORKER, 1);
    sem_t *working_tables = sem_create(WORKING_TABLE, 2);
    // print out the workers calling lines and wait for them (user initiated)
    wait_for_workers(order, 3, 3);
    // go to the chef behaviour
    chef_behaviour(order, ingr, 3, working_tables, mantime);
    
    // close the store (dettach everything and release them properly)
    ShmPair shm_table[] = {order};
    close_store(children_done, available_resources, ingr, 3, shm_table, 1);
    sem_clear(WORKING_TABLE, working_tables);
    sem_clear(MUTEX, mutex);
    fclose(logfile);
    exit(0);
}
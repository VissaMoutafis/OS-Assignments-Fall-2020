// Chef Code Base by Vissarion Moutafis

#include "Chef.h"
#include "ParsingUtils.h"

char* available_resources[] = {TOMATO, ONION, PEPPER};
sem_t *mutex;

static int get_int_in(int l, int h) {
    if (l == h) h++;

    int n = l - 1;
    do {
        n = rand() % h;
    } while(n < l || n > h);

    return n;
}

static void wait_for_workers(Order order, int num_of_workers, int num_of_resources) {
    printf("Ready to call the salad-maker programs. \
    Call them with the following command line instructions:\n");
    for (int i = 0; i < num_of_workers; ++i) {
        int t1, t2;
        t1 = get_int_in(0, MAX_PREP_TIME-1);
        t2 = get_int_in(t1, MAX_PREP_TIME);
        printf("./salad-maker -t1 %d -t2 %d -s %d -i %s", t1, t2, order.salad_counter_id, available_resources[i%num_of_resources]);
        if (i != num_of_workers)
            printf(" & ");
        else 
            printf("\n");
    }
    printf("\nPress any key, when ready: ");
    getchar();
}

static void provide_ingredients(Ingredients ingr[], int size) {
    int ingr_id1 = rand() % size;
    int ingr_id2;
    do {
        ingr_id2 = rand() % size;
    } while (ingr_id1 == ingr_id2);

    // sem_P(mutex);

    //start of critical section (CS)
    // provide the salad makers with 2 ingredients
    int value=-100;
    sem_V(ingr[ingr_id1]);
    sem_getvalue(ingr[ingr_id1], &value);
    printf("1.Provided with resource '%s' %d, \n", available_resources[ingr_id1],value);
    
    sem_V(ingr[ingr_id2]);
    sem_getvalue(ingr[ingr_id2], &value);
    printf("2.Provided with resource '%s' %d, \n", available_resources[ingr_id2], value);

    // end of critical section
    // sem_V(mutex);
}

static void take_a_break(int mantime) {
    // basic sleep
    sleep(mantime);
}

static bool check_done(Order order) {
    // we need to check everything atomicaly
    sem_P(mutex);
    int n = *(int*)(order.salad_counter);
    sem_V(mutex);

    return n <= 0;
}

static void chef_behaviour(Order order, Ingredients* ingr, int ingr_size, int mantime) {
    // TODO : pick 2 ingredients, wait for a little. Repeat
    while (!check_done(order)){
        provide_ingredients(ingr, ingr_size);
        take_a_break(mantime);
        // *(int*)(order.salad_counter) += -1;
    }
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
        shm_destroy(shm_table[i].salad_counter_id);
}


static int create_order(Order *order, int number_of_salads) {
    // create the shared memory
    order->salad_counter_id = shm_create(sizeof(int));
    if (order->salad_counter_id < 0)
        return ORDER_FAILURE_CRT;
    printf("Memmory ID: %d\n", order->salad_counter_id);
    // if everything ok, then attach the process to the shared memory part
    order->salad_counter = shm_attach(order->salad_counter_id);
    // initialize the shared segment
    *(int*)(order->salad_counter) = number_of_salads;

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
    srand((unsigned int) time(NULL));

    char *proper_args[] = {"-n", "-m"};
    char **parsed = NULL;    

    bool ret_val = parse_args(argc, argv, proper_args, 2, proper_args, 2, &parsed);
    
    if (ret_val == false)
        exit(1);
    
    
    // transform the arguments to int
    int number_of_salads = atoi(parsed[0]);
    int mantime = atoi(parsed[1]);
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

    printf("The shared mem has value: %d\n", *(int*)(order.salad_counter));
    wait_for_workers(order, 3, 3);
    chef_behaviour(order, ingr, 3, mantime);
    
    // close the store (dettach everything and release them properly)
    ShmPair shm_table[] = {order};
    close_store(children_done, available_resources, ingr, 3, shm_table, 1);
    sem_clear(MUTEX, mutex);
    exit(0);
}
#include "Salad-Maker.h"

#include "ParsingUtils.h"

int t1 = -1, t2 = -1;
char ingredient_name[20];
sem_t *mutex, *log_mutex;
char *salad_maker_name;
int saladmaker_index = -1;
FILE *logfile, *common_log;

static void get_index(char *ingr_name) {
    if (strcmp(ingr_name, TOMATO) == 0)
        saladmaker_index = tomato;
    else if (strcmp(ingr_name, ONION) == 0)
        saladmaker_index = onion;
    else 
        saladmaker_index = pepper;
}

static Ingredients set_ingredient_semaphores(char *ingr_name) {
    return sem_retrieve(ingr_name);
}

static bool check_done(Order order) {
    // we need to check everything atomicaly
    sem_P(mutex);
    int n = order.shmem->num_of_salads;
    sem_V(mutex);

    return n <= 0;
}

static void require_ingredient(Order order, Ingredients ingredient) {
    char log_buf[100];
    sprintf(log_buf, "Waiting for ingredients");
    print_log(log_code_wait_ingr, logfile, salad_maker_name, log_buf, NULL);
    print_log(log_code_wait_ingr, common_log, salad_maker_name, log_buf, log_mutex);

    while (!check_done(order) && sem_P_nonblock(ingredient) < 0);
    if (!check_done(order)) {
        sem_print(salad_maker_name, ingredient);

        sprintf(log_buf, "Get Ingredients");
        print_log(log_code_receive_ingr, logfile, salad_maker_name, log_buf, NULL);
        print_log(log_code_receive_ingr, common_log, salad_maker_name, log_buf, log_mutex);
    }
}

static void cook_salad(int time) {
    char log_buf[100];
    sprintf(log_buf, "Start cooking salad");
    print_log(log_code_cook_start, logfile, salad_maker_name, log_buf, NULL);
    print_log(log_code_cook_start, common_log, salad_maker_name, log_buf, log_mutex);
    // cook (idle time for the process)
    sleep(time);
    
    sprintf(log_buf, "Finished cooking salad.(%d s)", time);
    print_log(log_code_cook_end, logfile, salad_maker_name, log_buf, NULL);
    print_log(log_code_cook_end, common_log, salad_maker_name, log_buf, log_mutex);
}

static void deliver_salad(Order *order) {
    if (check_done(*order)) return;
    sem_P(mutex);
    // start of the critical section
    order->shmem->num_of_salads -= 1;
    order->shmem->salads_per_saladmaker[saladmaker_index] += 1;
    // end of the critical section
    sem_V(mutex);
}

// function to acquire the shared memory parts
static void acquire_order(int shmid, Order *order) {
    assert(shmid > -1);
    // get the shmid and the relative shared memory segment
    order->shm_id = shmid;
    order->shmem = shm_attach(shmid);
}

// classis usage prompt
static void print_usage(void) {
    fprintf(stderr,
            "\n   Usage: ~$ ./salad-maker -t1 [lb] -t2 [ub] -s [shared memory "
            "id] -i [ingredient]\n");
}

// function to check and parse the arguments
static bool parse_args(int argc, char *argv[], char *proper_args[],
                       int proper_args_size, char *num_args[],
                       int num_args_size, char ***parsed) {
    // we assume the user allocated the proper memory for the parsed array

    if (check_args(argc, argv, proper_args, proper_args_size, num_args,
                   num_args_size) == false) {
        print_usage();
        return false;
    }
    *parsed = calloc(proper_args_size, sizeof(char *));

    for (int i = 1; i < argc; i += 2) {
        int id = find_arg_index(proper_args, proper_args_size, argv[i]);
        assert(id > -1);
        // assign the value

        (*parsed)[id] = argv[i + 1];
    }

    return true;
}


// call as ./salad-maker -t1 lb -t2 ub -s [shared mem id] -i [main resource string-like name]
int main(int argc, char *argv[]) {
    // initiaze the rand's seed
    srand((unsigned int)time(NULL));

    // initialize and check the argument list
    char *proper_args[] = {"-t1", "-t2", "-s", "-i"};
    char *numeric_args[] = {"-t1", "-t2", "-s"};
    char **parsed = NULL;
    bool ret_val = parse_args(argc, argv, proper_args, 4, numeric_args, 3, &parsed);
    if (ret_val == false) exit(1);
    // get the argument values
    t1 = atoi(argv[2]);
    t2 = atoi(argv[4]);
    if (t1 < 0) t1 = 0;
    if (t2 < 0) t2 = 0;
    if (t1 == t2) t2++;

    // acquire the order (shared mem segment)
    Order order;
    acquire_order(atoi(argv[6]), &order);

    // acquire the ingredient semaphores
    strcpy(ingredient_name, argv[8]);
    if (strcmp(ingredient_name, TOMATO) == 0)
        salad_maker_name = "Saladmaker1";
    else if (strcmp(ingredient_name, ONION) == 0)
        salad_maker_name = "Saladmaker2";
    else
        salad_maker_name = "Saladmaker3";

        // open the log files
    char filename[100] = "./logs/";
    strcat(filename, salad_maker_name);
    logfile = fopen(filename, "w");         // personal log file
    common_log = fopen(LOG_PATH, "a");      // common logfile
    
    // start by printing the process id to the personal proc logfile
    char b[100];
    sprintf(b, "%d", getpid());
    print_log(log_code_print_pid, logfile, salad_maker_name, b, NULL);

    Ingredients ingr = set_ingredient_semaphores(ingredient_name);
    if (ingr == NULL)
        perror("retrieving sems at workers");
    // acquire the mutex
    mutex = sem_retrieve(MUTEX);
    log_mutex = sem_retrieve(LOG_MUTEX);
    // acquire the table semaphore so that you signal the chef that everything is ok 
    sem_t * table = sem_retrieve(WORKING_TABLE);
    // get the salad makers indexing based on the realtive enum type (in order to increase the proper salad counter in the shared memory)
    get_index(ingredient_name);


    do {
        // request ingredients
        require_ingredient(order, ingr);
        // inform that you took the ingredients
        sem_V(table); 
        // if there are still salads to be cooked
        if (!check_done(order)) {
            // cook the salad for some time
            cook_salad(get_int_in(t1, t2));
            // deliver the salad
            deliver_salad(&order);                
        }        
    } while (!check_done(order));
    
    // make sure you submit that you're done
    // increase the counter of done workers
    sem_P(mutex);
    order.shmem->num_of_finished += 1;
    sem_V(mutex);

    // dettach process from the semaphores
    sem_dettach(ingr);
    sem_dettach(mutex);
    sem_dettach(log_mutex);
    sem_dettach(table);
    free(parsed);
    fclose(logfile);
    fclose(common_log);
    exit(0);
}
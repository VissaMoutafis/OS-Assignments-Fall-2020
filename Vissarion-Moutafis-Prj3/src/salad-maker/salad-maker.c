#include "Salad-Maker.h"

#include "ParsingUtils.h"

int t1 = -1, t2 = -1;
char ingredient_name[20];
sem_t *mutex;
char *in;
int saladmaker_index = -1;
FILE * logfile;

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
    while (!check_done(order) && sem_P_nonblock(ingredient) < 0);
    if (!check_done(order))
        sem_print(in, ingredient);
}

static void cook_salad(int time) {
    sleep(time);
    fprintf(logfile, "Salad ready! (work time: %d seconds)\n", time);
}

static void deliver_salad(Order *order) {
    if (check_done(*order)) return;
    sem_P(mutex);
    // start of the critical section
    order->shmem->num_of_salads -= 1;
    // end of the critical section
    sem_V(mutex);
}

static void acquire_order(int shmid, Order *order) {
    assert(shmid > -1);

    order->shm_id = shmid;
    order->shmem = shm_attach(shmid);
}

static void print_usage(void) {
    fprintf(stderr,
            "\n   Usage: ~$ ./salad-maker -t1 [lb] -t2 [ub] -s [shared memory "
            "id] -i [ingredient]\n");
}

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

int main(int argc, char *argv[]) {
    srand((unsigned int)time(NULL));

    char *proper_args[] = {"-t1", "-t2", "-s", "-i"};
    char *numeric_args[] = {"-t1", "-t2", "-s"};
    char **parsed = NULL;

    bool ret_val =
        parse_args(argc, argv, proper_args, 4, numeric_args, 3, &parsed);

    if (ret_val == false) exit(1);

    t1 = atoi(argv[2]);
    t2 = atoi(argv[4]);
    if (t1 < 0) t1 = 0;
    if (t2 < 0) t2 = 0;
    if (t1 == t2) t2++;
    char filename[100] = "./logs/saladmaker-";
    strcat(filename, argv[argc-1]);
    logfile = fopen(filename, "w");

    Order order;
    acquire_order(atoi(argv[6]), &order);

    // acquire the ingredient semaphores
    strcpy(ingredient_name, argv[8]);
    in =  ingredient_name;
    get_index(ingredient_name);
    Ingredients ingr = set_ingredient_semaphores(ingredient_name);
    if (ingr == NULL)
        perror("retrieving sems at workers");
    // acquire the mutex
    mutex = sem_retrieve(MUTEX);
    sem_t *card = sem_retrieve(SALAD_WORKER);
    sem_t * table = sem_retrieve(WORKING_TABLE);

    fprintf(logfile, "Salad-maker-%s starts working...\n", ingredient_name);
    // check working card
    sem_P_nonblock(card);
    do {
        // request ingredients
        require_ingredient(order, ingr);
        if (!check_done(order)) {
            // get to the table
            sem_P_nonblock(table);
            // cook the salad for some time
            cook_salad(get_int_in(t1, t2));
            // deliver the salad
            deliver_salad(&order);
            if (!check_done(order))
                order.shmem->salads_per_saladmaker[saladmaker_index] += 1;
            sem_V(table);
        }
        // release the table
        
    } while (!check_done(order));
    // check out 
    sem_V(card);

    // dettach process from the semaphores
    sem_dettach(ingr);
    sem_dettach(card);
    sem_dettach(mutex);
    sem_dettach(table);
    free(parsed);
    exit(0);
}
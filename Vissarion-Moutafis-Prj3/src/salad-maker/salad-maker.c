#include "Salad-Maker.h"
#include "ParsingUtils.h"

int t1=-1, t2=-1;
char ingredient_name[20];
sem_t *mutex;
char* in[2];

static int get_int_in(int l, int h) {
    if (l == h) h++;
    
    int n = l - 1;
    do {
        n = rand() % h;
    } while(n < l || n > h);

    return n;
}

static void set_ingredient_semaphores(Ingredients ingr[], char *not_needed_ingr) {
    char* ingredients[] = {TOMATO, ONION, PEPPER};
    int j = 0;
    for (int i = 0; i < 3; i++) {
        if (strcmp(not_needed_ingr, ingredients[i]) != 0){
            in[j] = ingredients[i];
            ingr[j++] = sem_retrieve(ingredients[i]);
        }
    }
}

static bool check_done(Order order) {
    // we need to check everything atomicaly
    sem_P(mutex);
    int n = *(int*)(order.salad_counter);
    sem_V(mutex);

    return n <= 0;
}

static void require_ingredient(Order order, Ingredients ingredients[], int size){
    // start of critical section
    for (int i = 0; !check_done(order) && i < size; i ++) {
        while (!check_done(order) && sem_P_nonblock(ingredients[i]) < 0);
        sem_print(in[i], ingredients[i]);
    }
    // end of critical section
}

static void cook_salad(int time) {
    printf("Cooking...");
    sleep(time);
    printf(" Done (%d sec)\n", time);
}

static void deliver_salad(Order *order) {
    if (check_done(*order))
        return;
    sem_P(mutex);
    // start of the critical section
    *(int*)(order->salad_counter) += -1;
    printf("Salad ready! (%d salads left)\n", *(int*)(order->salad_counter));
    // end of the critical section
    sem_V(mutex);
}

static void acquire_order(int shmid, Order *order) {
    assert(shmid > -1);

    order->salad_counter_id =  shmid;
    order->salad_counter = shm_attach(shmid);
}

static void print_usage(void) {
    fprintf(stderr, "\n   Usage: ~$ ./salad-maker -t1 [lb] -t2 [ub] -s [shared memory id] -i [ingredient]\n");
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

int main(int argc, char* argv[]) {
    srand((unsigned int) time(NULL));

    char *proper_args[] = {"-t1", "-t2", "-s", "-i"};
    char *numeric_args[] = {"-t1", "-t2", "-s"};
    char **parsed = NULL;    

    bool ret_val = parse_args(argc, argv, proper_args, 4, numeric_args, 3, &parsed);
    
    if (ret_val == false)
        exit(1);

    t1 = atoi(argv[2]);
    t2 = atoi(argv[4]);
    if (t1 < 0) t1 = 0;
    if (t2 < 0) t2 = 0;
    if (t1 == t2) t2++;

    Order order; 
    acquire_order(atoi(argv[6]), &order);

    // acquire the ingredient semaphores
    strcpy(ingredient_name, argv[8]);
    Ingredients ingr[2] = {NULL, NULL};
    set_ingredient_semaphores(ingr, ingredient_name);
    
    // acquire the mutex
    mutex = sem_retrieve(MUTEX);
    sem_t *card = sem_retrieve(SALAD_WORKER);
    sem_P_nonblock(card);
    do {
        require_ingredient(order, ingr, 2);
        if (!check_done(order)) {
            cook_salad(get_int_in(t1, t2));
            deliver_salad(&order);
        }
    } while (!check_done(order));
    sem_V(card);

    sem_dettach(ingr[0]);
    sem_dettach(ingr[1]);
    sem_dettach(card);
    sem_dettach(mutex);
    free(parsed);
    exit(0);
}
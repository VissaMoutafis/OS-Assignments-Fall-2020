/*
** Worker processes code base
** Implemented by Vissarion Moutafis
*/

#include "Process.h"
#include "ParsingUtils.h"
#include <time.h>

static char *posible_algos[] = {
    "./primes1",
    "./primes2",
    "./primes3",
};
int root_pid=-1;

void worker_handler(int sig, siginfo_t *siginfo, void *context) {
    // exit process
    exit(0);
}

static void check_args(int argc, char* argv[]) {
    if (argc != 10) {
        fprintf(stderr, "Wrong input! ./workers -l [min] -u [max] -algo [algo num] -ripd [root pid] [child id]\n");
        exit(1);
    }
    if( !( is_numeric(argv[2]) && is_numeric(argv[4]) && is_numeric(argv[6]) && is_numeric(argv[8])) ) {
        fprintf(stderr, "All args must be numeric.\n");
        exit(1);
    }
    if(!(atoi(argv[6]) <= PRIME_ALGOS)) {
        fprintf(stderr, "The algorithm inedx must be between 0 and %d.\n", PRIME_ALGOS-1);
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    check_args(argc, argv);
    root_pid = atoi(argv[8]);

    // for the process that will execute
    pid_t child_pid = fork();
    if (child_pid == -1) {
        perror("fork");
        exit(1);
    }
    
    if(child_pid == 0) {
        // child case

        // choose the algorithm
        char *bin_path = posible_algos[atoi(argv[6])];
        //execute the program
        if (execl(bin_path, bin_path, argv[2], argv[4], (char *)0) == -1) {
            perror("execvp()");
            exit(1);
        }
    }
    

    // wait for the child to end
    if ((child_pid = wait(NULL)) == -1) {
        char msg[20];
        sprintf(msg, "wait prime%s", argv[6]);
        perror(msg);
        exit(1);
    }
    char msg[20];
    // print the child index for the time profiling
    sprintf(msg, "%s:", argv[9]);
    write(STDOUT_FILENO, msg, strlen(msg));

    // ask parent to end stuff
    #ifdef PROTOCOL
    wait_signal_from(root_pid, SIGUSR1, worker_handler);
    #else
    send_signal_to(root_pid, SIGUSR1, worker_handler);
    #endif

    exit(0);
}
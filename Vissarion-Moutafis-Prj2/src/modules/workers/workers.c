#include "Process.h"
#include "ParsingUtils.h"
#include <time.h>

static char *posible_algos[] = {
    "./primes1",
    "./primes2",
    "./primes3",
};



// argv = min, max, algorithm index
int main(int argc, char* argv[]) {
    srand(time(NULL));
    if (argc != 7) {
        fprintf(stderr, "Wrong input! ./workers -l min -u max -algo algo num\n");
        exit(1);
    }
    if( !( is_numeric(argv[2]) && is_numeric(argv[4]) && is_numeric(argv[6]) && atoi(argv[6]) <= PRIME_ALGOS) ) {
        fprintf(stderr, "All args must be numeric.\n");
        exit(1);
    }

    int fd[2];
    if (pipe(fd) == -1) {
        perror("pipe()");
        exit(1);
    }
    // close the pipe write-end for parent
    close(fd[WRITE]);

    pid_t child_pid = fork();
    if(child_pid == 0) {
        // child case
        
        // close the pipe read-end for child
        close(fd[READ]);

        // choose the algorithm
        char *bin_path = posible_algos[atoi(argv[6])];
        char *args[] = {bin_path, argv[2], argv[4], (char *)0};
        //execute the program
        if (execvp(bin_path, args) == -1) {
            perror("execvp()");
            exit(1);
        }
    }

    // wait for the child to end
    if (child_pid == wait(NULL))
        print_primes_from_child(fd[READ]);
    else {
        perror("wait for prime algo");
        exit(1);
    } 

    kill(getppid(), SIGUSR1); //send sigusr1 signal to the parent
    exit(0);
}
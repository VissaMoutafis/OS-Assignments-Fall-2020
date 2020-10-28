/*
** Implemented by Vissarion Moutafis
*/

#include "Process.h"
#include "ParsingUtils.h"
#include "Types.h"

void close_sibl_pipes(int **fd_board, int child_index, int num_of_children) {
    for (int k = 0; k < child_index; k++) {
        if (child_index != k) {
            close(fd_board[k][WRITE]);
            close(fd_board[k][READ]);
        }
    }
}

void child_behaviour(char** args) {
    if (execvp("./workers", args) == -1) {
        perror("execvp()");
        exit(1);
    }
}

void handler() {
    signal(SIGUSR1, handler);
    kill(getppid(), SIGUSR1); // send a usr1 to proccess-tree root
}

void parent_behaviour(int **fd_board, int num_of_children) {
    for (int i = 0; i < num_of_children; ++i) {
        print_primes_from_child(fd_board[i][READ]);
    }
}

void create_workers(int num_of_children, Range* ranges) {
    int **fd_board = calloc(num_of_children, sizeof(int*)); // this is the board with the file descriptors for each child
    for (int i = 0; i < num_of_children; i++) 
        fd_board[i] = calloc(2, sizeof(int));

    for (int i = 0; i < num_of_children; i++) {
        char algo_index[5];
        sprintf(algo_index, "%d", i%PRIME_ALGOS);
        // create communication pipe
        if (pipe(fd_board[i]) == -1) {
            perror("pipe");
            exit(1);
        }

        // create hte child process
        pid_t child_pid = fork();
        if (child_pid == -1) {
            perror("fork");
            exit(1);
        }

        // if it's a child
        if (child_pid == 0) {
            // we first close the siblings' pipes
            close_sibl_pipes(fd_board, i, num_of_children);
            // close the reading side of the pipe
            close(fd_board[i][READ]);                
            // create the arg list and execute the external node code
            char *args[] = {"./workers", "-l", ranges[i].l, "-u", ranges[i].u, "-algo", algo_index, (char *)0};
            child_behaviour(args);
        }
    }
    
    for (int i = 0; i < num_of_children; ++i) {
        close(fd_board[i][WRITE]); // close all write fd's
    }
    parent_behaviour(fd_board, num_of_children);
}


int main(int argc, char* argv[]) {
    if (argc != 7) {
        fprintf(stderr, "Wrong input! ./internal -l min -u max -w num-of-children.\n");
        exit(1);
    }
    
    if (atoi(argv[2]) > atoi(argv[4])) {
        fprintf(stderr, "Wrong input! ./internal -l min -u max -w num-of-children.\n");
        exit(1);
    }

    // set the 
    signal(SIGUSR1, handler);
    
    internal_node_behaviour(argc, argv, create_workers);

    exit(0);
}


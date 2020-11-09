/*
** Implemented by Vissarion Moutafis
*/

#include "Process.h"
#include "Types.h"

char* w;
int n;
volatile sig_atomic_t signals_encountered = 0;
sig_atomic_t *pids_visited;

void root_handler (int sig, siginfo_t *siginfo, void *context) {
    volatile sig_atomic_t pid = siginfo->si_value.sival_int;
    if (pids_visited[pid%(n*n)] == 0){
        signals_encountered = signals_encountered + 1;
        // printf("Caught %d\n", pid);
        pids_visited[pid%(n*n)] = 1;
        kill(pid, SIGUSR1);
    }
}

void child_behaviour(char** args, int fd_board[][2], int i, int num_of_children) {
    // we begin with some initializations

    // we first close the siblings' pipes
    close_sibl_pipes(fd_board, i, num_of_children);
    // make sure that the child will print the out put to the pipe's write-end
    close(fd_board[i][READ]);
    //duplicate the write descriptor to stdout so we catch the result
    if (dup2(fd_board[i][WRITE], STDOUT_FILENO) == -1) {
        perror("dup2 in root");
        exit(1);
    }
    close(fd_board[i][WRITE]);

    if (execvp("./internals", args) == -1) {
        perror("execvp()");
        exit(1);
    }
}

void catch_usr1(int timeouts) {
    // float sec_wait = 0.7;
    while(signals_encountered < timeouts*timeouts-2){
    }
}

void parent_behaviour(int fd_board[][2], int num_of_children) {
    // read and print the appropriate messages
    // catch_usr1(num_of_children);
    internal_read_from_child(fd_board, num_of_children);
    set_signal_handler(SIGUSR1, root_handler);
    // catch_usr1(num_of_children);
}

void create_internals(int num_of_children, Range* ranges) {
    int fd_board[num_of_children][2]; // this is the board with the file descriptors for each child
    for (int i = 0; i < num_of_children; i++) {
        // create communication pipe
        if (pipe(fd_board[i]) == -1) {
            perror("pipe()");
            exit(1);
        }

        // create the child process
        pid_t child_pid = fork();
        if (child_pid == -1) {
            perror("fork");
            exit(1);
        }

        // if it's a child
        if (child_pid == 0) { 
            // set the algo index in a string
            char algo_index[5];
            sprintf(algo_index, "%d", i*num_of_children);
            char* args[] = {"./internals", 
                            "-l", ranges[i].l, 
                            "-u", ranges[i].u, 
                            "-w", w, 
                            "-i", algo_index, 
                            (char*)0};
            child_behaviour(args, fd_board, i, num_of_children);
        }
    }

    for (int i = 0; i < num_of_children; ++i) {
        close(fd_board[i][WRITE]); // close all write fd's
    }

    parent_behaviour(fd_board, num_of_children);
}
void check_args(int argc, char* argv[]) {
    if (argc != 7) {
        fprintf(stderr, "Wrong input! ./root -l min -u max -w num-of-children.\n");
        exit(1);
    }
    
    if (atoi(argv[2]) > atoi(argv[4])) {
        fprintf(stderr, "Wrong input! ./root -l min -u max -w num-of-children.\n");
        exit(1);
    }
}
int main(int argc, char* argv[]) {
    check_args(argc, argv);
    set_signal_handler(SIGUSR1, root_handler);

    w = argv[6];
    
    n = atoi(w);
    pids_visited = calloc(n*n, sizeof(int)); // max amount of different signals are n*n
    
    for (int i = 0; i < n*n; i++)
        pids_visited[i] = 0;
    
    printf("\nPID %d\n", getpid());
    internal_node_behaviour(argc, argv, create_internals);
    
    printf("Total SIGUSR1 signals received: %d\n", signals_encountered);
    free(pids_visited);
    exit(0);
}

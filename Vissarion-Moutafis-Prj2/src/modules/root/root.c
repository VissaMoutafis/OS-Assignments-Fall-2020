/*
** Implemented by Vissarion Moutafis
*/

#include "Process.h"
#include "Types.h"

char* w;
volatile sig_atomic_t signals_encountered = 0;

void root_handler (int sig, siginfo_t *siginfo, void *context) {
    signals_encountered++;
    pid_t pid = siginfo->si_value.sival_int;
    printf("root Caught %d\n", pid);
    kill(pid, SIGUSR1);
}

void child_behaviour(char** args) {
    if (execvp("./internals", args) == -1) {
        perror("execvp()");
        exit(1);
    }
}

void parent_behaviour(int fd_board[][2], int num_of_children) {
    // read and print the appropriate messages
    internal_read_from_child(fd_board, num_of_children);
    // for (int i = 0; i < num_of_children; i++)
    //     print_primes_from_child(fd_board[i][READ]);
}

void create_internals(int num_of_children, Range* ranges) {
    int fd_board[num_of_children][2]; // this is the board with the file descriptors for each child
    for (int i = 0; i < num_of_children; i++) {
        // create communication pipe
        if (pipe(fd_board[i]) == -1) {
            perror("pipe()");
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
            
            close(fd_board[i][READ]);   
            dup2(fd_board[i][WRITE], STDOUT_FILENO); 
            close(fd_board[i][WRITE]); 

            char* args[] = {"./internals", "-l", ranges[i].l, "-u", ranges[i].u, "-w", w, (char*)0};
            child_behaviour(args);
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
    printf("\nPID %d\n", getpid());
    internal_node_behaviour(argc, argv, create_internals);
    
    exit(0);
}

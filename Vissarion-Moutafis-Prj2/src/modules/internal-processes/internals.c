/*
** Implemented by Vissarion Moutafis
*/

#include "Process.h"
#include "ParsingUtils.h"
#include "Types.h"



void child_behaviour(char** args) {
    if (execvp("./workers", args) == -1) {
        perror("execvp()");
        exit(1);
    }
}
void internal_handler(int sig, siginfo_t *siginfo, void *context) {
    return;
}

void kill_children(pid_t children_pid[], int num_of_children) {
    set_signal_handler(SIGUSR1, internal_handler);    
    
    for (int i = 0; i < num_of_children; i++) {
        // send the signal to the child
        printf("sending usr1 to child %d", children_pid[i]);
        kill(children_pid[i], SIGUSR1);
        // wait for root to tell you that is done
        wait_signal_from_parent(internal_handler);
    }
}

void parent_behaviour(pid_t children_pid[], int fd_board[][2], int num_of_children) {
    // read and print the appropriate messages
    internal_read_from_child(fd_board, num_of_children);
    // we are done reading so we must send kill signals to all the children
    // kill_children(children_pid, num_of_children);
}

void create_workers(int num_of_children, Range* ranges) {
    // this is the roots pid
    char root_pid[20];
    sprintf(root_pid, "%d", getppid());

    // this is the board with the file descriptors for each child
    int fd_board[num_of_children][2];
    pid_t children_pid[num_of_children];
   
    // initialize the array of children pids
    for (int i = 0; i < num_of_children; i++)
        children_pid[i] = -1;

    for (int i = 0; i < num_of_children; i++) {
        char algo_index[5];
        sprintf(algo_index, "%d", i%PRIME_ALGOS);
        // create communication pipe
        if (pipe(fd_board[i]) == -1) {
            perror("pipe");
            exit(1);
        }

        // create the child process
        pid_t child_pid = fork();
        if (child_pid == -1) {
            perror("fork");
            exit(1);
        }

        // add child's pid to the pid table for later use
        children_pid[i] = child_pid;

        // if it's a child
        if (child_pid == 0) {
            // we first close the siblings' pipes
            close_sibl_pipes(fd_board, i, num_of_children);
            // close the reading side of the pipe
            close(fd_board[i][READ]);   
            dup2(fd_board[i][WRITE], STDOUT_FILENO); 
            close(fd_board[i][WRITE]);            
            // create the arg list and execute the external node code
            char *args[] = {"./workers", "-l", ranges[i].l, "-u", ranges[i].u, "-algo", algo_index, "-rootpid", root_pid, (char *)0};
            child_behaviour(args);
        }
    }
    
    for (int i = 0; i < num_of_children; ++i) {
        close(fd_board[i][WRITE]); // close all write fd's
    }
    parent_behaviour(children_pid, fd_board, num_of_children);
}

static void check_args(int argc, char* argv[]) {
    if (argc != 7) {
        fprintf(stderr, "Wrong input! ./internal -l min -u max -w num-of-children.\n");
        exit(1);
    }
    
    if (atoi(argv[2]) > atoi(argv[4])) {
        fprintf(stderr, "Wrong input! ./internal -l min -u max -w num-of-children.\n");
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    check_args(argc, argv);

    internal_node_behaviour(argc, argv, create_workers);

    exit(0);
}
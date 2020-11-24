/*
** Internal process codebase
** Implemented by Vissarion Moutafis
*/

#include "Process.h"
#include "ParsingUtils.h"
#include "Types.h"

int algo_index = 0;

void child_behaviour(char** args, int fd_board[][2], int i, int num_of_children) {
    // we first close the siblings' pipes
    close_sibl_pipes(fd_board, i, num_of_children);
    // make the write-end non blocking
    // make sure that the child will print the out put to the pipe's write-end
    close(fd_board[i][READ]);
    // duplicate stdout so parent catch all the stuff the child writes
    if (dup2(fd_board[i][WRITE], STDOUT_FILENO) == -1) {
        perror("dup2 in internal processes");
        exit(1);
    }
    close(fd_board[i][WRITE]);
    if (execvp("./workers", args) == -1) {
        perror("execvp()");
        exit(1);
    }
}


void parent_behaviour(int fd_board[][2], int num_of_children) {
    // read and print the appropriate messages
    internal_read_from_child(fd_board, num_of_children, internal);
}

void create_workers(int num_of_children, Range* ranges) {
    // this is the roots pid
    char root_pid[20];
    sprintf(root_pid, "%d", getppid());

    // this is the board with the file descriptors for each child
    int fd_board[num_of_children][2];
   
    for (int i = 0; i < num_of_children; i++) {
        char algo[5];
        sprintf(algo, "%d", algo_index%PRIME_ALGOS);
        char child_id[20];
        sprintf(child_id, "%d", algo_index);
    
        algo_index++;
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

        // if it's a child
        if (child_pid == 0) {
            
            // create the arg list and execute the external node code
            char *args[] = {"./workers", 
                            "-l", ranges[i].l, 
                            "-u", ranges[i].u, 
                            "-algo", algo, "-rootpid", root_pid, child_id, (char *)0};
            child_behaviour(args, fd_board, i, num_of_children);
            exit(1);
        }
    }
    
    for (int i = 0; i < num_of_children; ++i) {
        close(fd_board[i][WRITE]); // close all write fd's
    }
    parent_behaviour(fd_board, num_of_children);
}

static void check_args(int argc, char* argv[]) {
    if (argc != 9) {
        fprintf(stderr, "Wrong input! ./internal -l min -u max -w num-of-children -i algo index.\n");
        exit(1);
    }
    
    if (atoi(argv[2]) > atoi(argv[4])) {
        fprintf(stderr, "Wrong input! ./internal -l min -u max -w num-of-children -i algo index.\n");
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    check_args(argc, argv);
    algo_index = atoi(argv[8]);
    internal_node_behaviour(argc, argv, create_workers);
    exit(0);
}
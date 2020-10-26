/*
** Implemented by Vissarion Moutafis
*/

#include "Process.h"
#include "Types.h"
#include <string.h>

char* w;

void close_sibl_pipes(int fd_board[][2], int child_index, int num_of_children) {
    for (int k = 0; k < num_of_children; k++) {
        if (child_index != k) {
            close(fd_board[k][WRITE]);
            close(fd_board[k][READ]);
        }
    }
}

void child_behaviour(char** args) {
    if (execvp("./internals", args) == -1) {
        perror("execvp()");
        exit(1);
    }
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
            
            close(fd_board[i][READ]);                // close the reading side of the pipe

            char* args[] = {"./internals", "-l", ranges[i].l, "-u", ranges[i].u, "-w", w, (char*)0};
            child_behaviour(args);
        }
    }
    
    for (int i = 0; i < num_of_children; ++i) {
        close(fd_board[i][WRITE]); // close all write fd's
        char msg[BUFSIZ];
        if (read(fd_board[i][READ], msg, BUFSIZ) > 0)
            printf("%s ", msg);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 7) {
        fprintf(stderr, "Wrong input! ./root -l min -u max -w num-of-children.\n");
        exit(1);
    }
    
    if (atoi(argv[2]) > atoi(argv[4])) {
        fprintf(stderr, "Wrong input! ./root -l min -u max -w num-of-children.\n");
        exit(1);
    }

    w = argv[6];

    internal_node_behaviour(argc, argv, create_internals);

    exit(0);
}

/*
** Implemented by Vissarion Moutafis
*/

#include "Process.h"
#include "Types.h"
#include <string.h>

void create_workers(int num_of_children, Range* ranges) {
    int fd_board[num_of_children][2];
    

    for (int i = 0; i < num_of_children; i++) {
        
        // create hte child process
        pid_t child_pid = fork();
        if (pipe(fd_board[i]) == -1) {
            perror("pipe()");
            exit(1);
        }
        dup2(fd_board[i][READ], 0);
        close(fd_board[i][READ]);

        // if it's a child
        if (child_pid == 0) {
            close(fd_board[i][READ]); // close the reading side of the pipe
            dup2(fd_board[i][WRITE], 1); // determine that the new write stream is stdou 
            close(fd_board[i][WRITE]);
            
            char* args[] = {"./workers", "-l", ranges[i].l, "-u", ranges[i].u, (char*)0};
            if (execvp("./workers", args) == -1) {
                perror("execvp()");
                exit(1);
            }
        }
    }

    for (int i = 0; i < num_of_children; ++i) {
        char msg[100];
        read(fd_board[i][READ], msg, 100);
        printf("Message from child number %d : %s\n", i+1, msg);
    }

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
    internal_node_behaviour(argc, argv, create_workers);

    exit(0);
}


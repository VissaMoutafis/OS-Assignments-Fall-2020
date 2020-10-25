/*
** Implemented by Vissarion Moutafis
*/

#include "Process.h"
#include "Types.h"
#include <string.h>

void create_workers(int num_of_children, Range* ranges) {
    for (int i = 0; i < num_of_children; i++) {
        // create hte child process
        pid_t child_pid = fork();
        
        // if it's a child
        if (child_pid == 0) { 
            char* args[] = {"./workers", "-l", ranges[i].l, "-u", ranges[i].u, (char*)0};
            if (execvp("./workers", args) == -1) {
                perror("execvp()");
                exit(1);
            }
        }
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


/*
** Implemented by Vissarion Moutafis
*/

#include "Process.h"
#include "Types.h"
#include <string.h>

char* w;

void create_internals(int num_of_children, Range* ranges) {
    for (int i = 0; i < num_of_children; i++) {
        // create hte child process
        pid_t child_pid = fork();
        
        // if it's a child
        if (child_pid == 0) { 
            char* args[] = {"./internals", "-l", ranges[i].l, "-u", ranges[i].u, "-w", w, (char*)0};
            if (execvp("./internals", args) == -1) {
                perror("execvp()");
                exit(1);
            }
        }
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

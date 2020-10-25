/*
** Implemented by Vissarion Moutafis
*/

#include "Process.h"
#include "Types.h"
#include <string.h>

void create_children(int num_of_children, Range* ranges) {
    for (int i = 0; i < num_of_children; i++) {
        // create hte child process
        pid_t child_pid = fork();

        if (child_pid == 0) { // if it's a child
            char* args[] = {"./workers", "-l", ranges[i].l, "-u", ranges[i].u, (char*)0};
            if (execvp("./workers", args) == -1) {
                perror("execvp()");
                exit(1);
            }
        }
    }
}

char* num_to_str(int num) {
    if (num == 0) return "0";

    char* str = calloc(1, sizeof(char));
    int len = 1;
    while (num) {
        char dig = num%10 + '0';
        num /= 10;

        char* new_str = calloc(++len, sizeof(char));
        strcpy(new_str+1, str);
        new_str[0] = dig;
        free(str);
        str = new_str;
    }

    return str;
}

// create n ranges from the interval [low, high]
static Range* divide_ranges(int n, int low, int high, int*size) {
    if (n<1) return NULL;
    
    // create the range board
    Range* range_board=calloc(n, sizeof(*range_board));

    int range_length = (high - low) / n;
    int start, end;
    start = low;
    end = low + range_length;
    int i = 0;
    *size = 0;
    do {
        (*size) ++;
        // create the range struct
        Range r = {num_to_str(start), num_to_str(end)};
        // insert the range in the range board
        range_board[i] = r;
        // insert the index
        i++;

        // update the range limits
        start = end + 1;
        end = (start + range_length) > high ? high : (start + range_length);
        // printf("Range: [%s, %s]\n", r.l, r.u);
    } while (start != high+1);

    return range_board;
}

void wait_children(void) {
    int pid, status;
    while ((pid = wait(&status)) != -1) {
        // printf("Child %d exited with status %d.\n", pid, status);
    }
    // printf("Parent %d exited\n", getpid());
}

void call_external(void) {

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

    // number of children processes
    int num_of_children = 0;
    num_of_children = atoi(argv[6]);
    
    // create the ranges that the workers will search for primes
    int size;
    Range *ranges = divide_ranges(num_of_children, atoi(argv[2]), atoi(argv[4]), &size);
    num_of_children = num_of_children > size ? size : num_of_children;  
    
    if (num_of_children > 0)
        create_children(num_of_children, ranges);

    // waiting for the children to exit
    wait_children();


    // free memory
    for (int i = 0; i < num_of_children; i++) {
        free(ranges[i].l);
        free(ranges[i].u);
    }
    free(ranges);

    exit(0);
}


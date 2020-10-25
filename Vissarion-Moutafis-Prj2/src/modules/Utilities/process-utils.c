#include "Process.h"
#include "ParsingUtils.h"
#include <string.h>
void wait_children(void) {
    int pid, status;
    while ((pid = wait(&status)) != -1) {
        // printf("Child %d exited with status %d.\n", pid, status);
    }
    // printf("Parent %d exited\n", getpid());
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


void internal_node_behaviour(int argc, char* argv[], CreateChildren create_children) {
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
}
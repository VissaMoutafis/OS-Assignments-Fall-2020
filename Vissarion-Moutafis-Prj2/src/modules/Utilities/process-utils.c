#include <limits.h>
#include "Process.h"
#include "ParsingUtils.h"
#include "PQ.h"

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
    } while (start != high+1);

    return range_board;
}

void print_primes_from_child(int fd) {
    char buffer[BUFSIZ];
    ssize_t error;
    if((error = read(fd, buffer, BUFSIZ)) != 0) {
        write(STDOUT_FILENO, buffer, strlen(buffer)); // print the buffer
        fflush(stdout);
    } else if (error == -1) {
        //something caused an error in the reading execution
        perror("read()");
        exit(1);
    }
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


typedef struct {
    int n;
    float time;
} *PrimePair;

int compare_pair(Pointer a, Pointer b) {
    // The input will be an integer
    return ((PrimePair)a)->n - ((PrimePair)b)->n;
}

Pointer make_pair(char* str) {
    PrimePair a = malloc(sizeof(*a));

    // parse the input
    int cols;
    char** str_pair = parse_line(str, &cols, ",");
    // set the values
    a->n = atoi(str_pair[0]);
    a->time = strtof(str_pair[1], NULL);
    // free the no longer needed memory
    free(str_pair[0]);
    free(str_pair[1]);
    free(str_pair);

    return (Pointer)a;
}

void visit_pair(Pointer p) {
    char buf[BUFSIZ];
    PrimePair a = (PrimePair)p;
    sprintf(buf, "%d,%.1f ", a->n, a->time);
    write(STDOUT_FILENO, buf, strlen(buf));
    fflush(stdout);
}

float get_timestamp(int fd) {
    char buffer[2];
    char *entry = calloc(1, sizeof(char));
    int len = 0;

    while (read(fd, buffer, 1) > 0) {
        buffer[1] = '\0';
        if (strcmp(buffer, ":") == 0)
            break;

        char *new_entry = calloc(++len + 1, sizeof(char));
        strcpy(new_entry, entry);
        free(entry);
        new_entry[len-1] = buffer[0];
        entry = new_entry;
    }

    float timestamp = strtof(entry, NULL);
    free(entry);

    return timestamp;
}

bool use_input(int fd, PQ pq, float* max_time, float* min_time) {
    char *entry = calloc(1, sizeof(char));
    int len = 0;
    char buffer[2]={'\0','\0'};
    bool end_file = false;
    while(read(fd, buffer, 1) > 0 && !end_file) {
        buffer[1] = '\0';
        if (strcmp(buffer, "$") == 0) {
            end_file = true;
        }
        if (strcmp(buffer, "\n") == 0)
            continue;

        // check for timestamps (min max)
        if (strcmp(buffer, ":") == 0) {
            float t = get_timestamp(fd);
            if ((t - (*min_time)) <= 0.1){
                *min_time = t;
            }
            if ((t - (*max_time)) >= 0.1){            
                *max_time = t;
            }
        } else if (strcmp(buffer, " ") != 0) {
            // build the number
            // we add an extra digit to the pqueue
            char *new_entry = calloc(++len + 1, sizeof(char));
            strcpy(new_entry, entry);
            free(entry);
            new_entry[len - 1] = buffer[0];
            entry = new_entry;
        } else if (len>0) {
            // we reached the end of a pair string
            // add the pair to the pqueue
            // perror(entry);
            pq_push(pq, make_pair(entry));
            // free the reserved memory since the string is no longer needed
            free(entry);
            // reset the string
            entry = calloc(1, sizeof(char));
            len = 0;
        }
    }
    free(entry); // there is at least 1 byte still-reachable from the last else case in the for loop

    return end_file;
}

void internal_read_from_child(int fd_array[][2], int number_of_children) {
    // make a pq to print the children sorted
    PQ pq = pq_create(compare_pair, free);
    float max_time=0, min_time = 9999999;
    bool has_active_children = false;
    struct pollfd fds[number_of_children];
    int open_files = number_of_children;
    //set up the fds array
    for (int i = 0; i < number_of_children; i++) {
        fds[i].events = POLLIN;
        fds[i].fd = fd_array[i][READ];
        fds[i].revents = 0;
    }
    
    // because poll might get interrupted we will ignore all usr1 signals
    ignore_signal(SIGUSR1);
    // volatile sig_atomic_t end_file;
    while(open_files) {
        // end_file = 0; // test if a file has reached to an end case

        // call the poll syscall
        int retpoll = poll(fds, number_of_children, TIMEOUT);

        if (retpoll > 0) {
            // if poll did not fail
            for (int i = 0; i < number_of_children; i++) {
                if (fds[i].fd != -1) {
                    // if the fd has something to read
                    if (fds[i].revents & POLLIN) {
                        // if the file descriptor is in the ready list
                        use_input(fds[i].fd, pq, &max_time, &min_time);
                        has_active_children = true;
                    }
                    if (fds[i].revents == POLLHUP){
                        fds[i].fd = -1;
                        open_files--;
                    }
                }
            }
        } else if (retpoll < 0) {
            perror("poll");
            exit(1);
        }
    }

    while (!pq_empty(pq)) {
        Pointer pair = pq_pop(pq);
        visit_pair(pair);
        fflush(stdout);
        free(pair);
    }
    if (has_active_children){ // if the children have actually wrote something
        char buf[BUFSIZ];
        sprintf(buf, ":%.1f::%.1f:\n", max_time, min_time);
        write(STDOUT_FILENO, buf, strlen(buf));
    }
    fflush(stdout);
    pq_destroy(pq);
}

void set_signal_handler(int signo, void (*handler)(int, siginfo_t *, void *)) {
    struct sigaction act;
    memset(&act, '\0', sizeof(act));
    act.sa_sigaction = handler;
    act.sa_flags = SA_SIGINFO;
    // set up signal handler 
    if (sigaction(signo, &act, NULL) < 0) {
		perror ("sigaction");
		exit(1);
	}
}

void ignore_signal(int signo) {
    struct sigaction act;
    memset(&act, '\0', sizeof(act));
    act.sa_handler = SIG_IGN;
    // set up signal handler 
    if (sigaction(signo, &act, NULL) < 0) {
		perror ("sigaction");
		exit(1);
	}
}

void wait_signal_from(pid_t receiver_pid, int signo, void (*handler)(int, siginfo_t *, void *)) {
    // set the signal handler
    set_signal_handler(signo, handler);
    // wait for parents signal
    int timeouts = 0;
    while (timeouts < 5000) {
        timeouts++;
        const union sigval val = {getpid()}; // make sure the receiver knows who send the signal
        if (sigqueue(receiver_pid, signo, val) < 0)
            exit(1);

        sleep(0.73);
    }
}

// function to close all the pipes from unrelated to the IO siblings
void close_sibl_pipes(int fd_board[][2], int child_index, int num_of_children) {
    for (int k = 0; k < child_index; k++) {
        if (child_index != k) {
            close(fd_board[k][WRITE]);
            close(fd_board[k][READ]);
        }
    }
}
/*
** General process utilities that more 
** that one process types will use.
** Implemented by Vissarion Moutafis
*/

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

float get_timestamp(int fd, int* w_id) {
    char buffer[2];
    char *entry = calloc(1, sizeof(char));
    int len = 0;
    char* proc_id = calloc(1, sizeof(char));
    int len_proc_id = 0;

    while (read(fd, buffer, 1) > 0) {
        buffer[1] = '\0';
        if (strcmp(buffer, ":") == 0)
            break;

        
        // between the first 2 collumns there is the timestamp
        char *new_entry = calloc(++len + 1, sizeof(char));
        strcpy(new_entry, entry);
        free(entry);
        new_entry[len-1] = buffer[0];
        entry = new_entry;
    }

    while (read(fd, buffer, 1) > 0) {
        buffer[1]='\0';
        if (strcmp(buffer, ":") == 0)
            break;
        //between the second and the third there is the proc id
        char *new_proc_id = calloc(++len_proc_id + 1, sizeof(char));
        strcpy(new_proc_id, proc_id);
        free(proc_id);
        new_proc_id[len_proc_id - 1] = buffer[0];
        proc_id = new_proc_id;
    }

    // cast the time stamp to float
    float timestamp = strtof(entry, NULL);
    free(entry);

    *w_id = atoi(proc_id);
    free(proc_id);
    return timestamp;
}

void use_input(int fd, PQ pq, float* max_time, float* min_time, float timestamps[], ProcessType proc_type) {
    char *entry = calloc(1, sizeof(char));
    int len = 0;
    char buffer[2]={'\0','\0'};
    while(read(fd, buffer, 1) > 0) {
        buffer[1] = '\0';

        if (strcmp(buffer, "\n") == 0)
            continue;

        // check for timestamps (min max)
        if (strcmp(buffer, ":") == 0) {
            // we reaced a timestamp that needs parsing
            int w_id=-1;                                // set the worker Id
            float t = get_timestamp(fd, &w_id);         // get the timestamp and the worker ID

            if (w_id > -1) {
                if (proc_type == root)                  // if you are the root
                    timestamps[w_id] = t;               // set the timestamp for the board to print later                   
                else {                                 // you are not the root, so pass it up the process tree
                    char buf[BUFSIZ];
                    sprintf(buf, ":%.1f:%d:", t, w_id);
                    write(STDOUT_FILENO, buf, strlen(buf));
                }
            } else {
                perror("False parsing timestamp");
                exit(1);
            }

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
            pq_push(pq, make_pair(entry));
            // free the reserved memory since the string is no longer needed
            free(entry);
            // reset the string
            entry = calloc(1, sizeof(char));
            len = 0;
        }
    }
    free(entry); // there is at least 1 byte still-reachable from the last else case in the for loop
}

static void write_batch(char** batch, int* size, int batch_size) {
    int bytes_to_write = *size;
    char* buf = *batch;
    if(write(STDOUT_FILENO, buf, bytes_to_write) == -1) {
        perror("writting the buffer");
        exit(1);
    }

    // Now that we wrote the buffer we need to reset it
    memset(*batch, '\0', batch_size);
    (*size) = 0;
}

static void send_batched_messages(void *messages, int* cnt) {
    PQ pq = *((PQ*)messages); // the messages are given in a priority queue
    int batch_len = BATCHSIZE;
    char *batch = calloc(batch_len, sizeof(char));

    // set a variable to count how many chars the batch-buffer contains
    int size = 0;

    // start batching till the pq is empty
    while (!pq_empty(pq)) {
        (*cnt)++;
        // pop one pair from the pq
        PrimePair pair = (PrimePair)pq_pop(pq); 

        // the length of timestamp
        int len_of_timestamp = get_len_of_int((int)pair->time) + 2;
        if (len_of_timestamp < 3)
            len_of_timestamp = 3;

        // for the integer call the relative utility
        int len_of_n = get_len_of_int(pair->n);
        // + 1 for the comma ',' character between +1 for the space char
        int len = len_of_n + len_of_timestamp + 1 + 1;
        // make the buffer
        char buf[len+1]; 
        // copy the string
        sprintf(buf, "%d,%.1f ", pair->n, pair->time);
        buf[len]='\0';

        // if the buffer's length is longer than the actual batch's length then change it
        if (len > batch_len){
            // first write to the output file what is left in the batch
            write_batch(&batch, &size, batch_len);
            // change the batch_len
            batch_len = len+1;
            // free the old memory and allocate new (initialized to 0)
            free(batch);
            batch = calloc(batch_len, sizeof(char));
        }

        // if the size + length of the buffer surpasses the length of the batch
        if (size + len >= batch_len) {
            // at this point we have to write the buffer to stdout
            write_batch(&batch, &size, batch_len);
        }

        // copy to the end of the previous batched buffer the new buffer entry
        strncpy(&batch[size], buf, len);
        // increase size
        size = size + len;
        // free the memory from the pair
        free(pair);
    } 

    // if there is anything left on the batch write it down
    if (size)
        write_batch(&batch, &size, batch_len);
    // de-allocate the memory of the batch
    free(batch);
}

static void print_workers_timestamps(float timestamps[], int size) {
    for (int i = 0;  i < size; i++)
        printf("Time for W%d: %.1f ms\n", i, timestamps[i]);
}

void internal_read_from_child(int fd_array[][2], int number_of_children, ProcessType proc_type) {
    // make a pq to print the children sorted
    int cnt = 0;
    // initialize the children timestamp array
    float children_timestamps[number_of_children * number_of_children];
    if (proc_type == root)
        for (int i = 0; i < number_of_children*number_of_children; i++)
            children_timestamps[i] = 0.0;

    // intialize the PQ to put the primes in
    PQ pq = pq_create(compare_pair, free);
    // max and min times for each process
    float max_time=0, min_time = 9999999;
    bool has_active_children = false;
    // initialize the poll fds board
    struct pollfd fds[number_of_children];
    // number of open files per node is the number of children
    int open_files = number_of_children;
    //set up the fds array
    for (int i = 0; i < number_of_children; i++) {
        fds[i].events = POLLIN;
        fds[i].fd = fd_array[i][READ];
        fds[i].revents = 0;
    }
    
    // because poll might get interrupted we will ignore all usr1 signals
    while(open_files) {
        // call the poll syscall
        int retpoll = poll(fds, number_of_children, 0);

        if (retpoll > 0) {
            // if poll did not fail
            for (int i = 0; i < number_of_children; i++) {
                if (fds[i].fd != -1) {
                    // if the fd has something to read
                    if (fds[i].revents & POLLIN) {
                        // if the file descriptor is in the ready list
                        use_input(fds[i].fd, pq, &max_time, &min_time, children_timestamps, proc_type);
                        has_active_children = true; // if this turn true then the node has active children
                    }
                    if (fds[i].revents == POLLHUP){
                        fds[i].fd = -1;
                        open_files--;
                    }
                }
            }
        } else if (retpoll < 0) {
            // for debbuging reasons 
            // perror("poll");
            // exit(1);
        }
    }
    
    // now that we are done with everything we have to print all the primes we got 
    send_batched_messages(&pq, &cnt);

    // actions to take if you are root
    if (proc_type == root) {
        char buf[BUFSIZ];
        sprintf(buf, "\nRoot found %d primes.\n", cnt);
        write(STDOUT_FILENO, buf, strlen(buf));

        if (has_active_children){ // if the children have actually wrote something
            char buf[BUFSIZ];
            sprintf(buf, "Min Time for Workers: %.1f ms\nMax Time for Workers: %.1f ms\n", min_time, max_time);
            write(STDOUT_FILENO, buf, strlen(buf));
            // print the worker times
            print_workers_timestamps(children_timestamps, number_of_children*number_of_children);
        }
    }

    fflush(stdout);
    pq_destroy(pq);
}

void set_signal_handler(int signo, void (*handler)(int, siginfo_t *, void *)) {
    // make the block set for sigaction 
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);

    struct sigaction act;
    // initialize the struct
    memset(&act, '\0', sizeof(act));
    act.sa_sigaction = handler;
    // we will use the sa_sigaction handler and 
    // we want the syscall to restart after interruption
    act.sa_flags = SA_SIGINFO | SA_RESTART;
    act.sa_mask = set;

    // set up signal handler 
    if (sigaction(signo, &act, NULL) < 0) {
        // sigaction failed
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
    float time = 0.1;
    float thresh = (float)(getpid()%100)/100.0 < 0.8 ? (float)(getpid()%100)/100.0 : 0.8;
    while (timeouts < 1000) {
        timeouts++;
        const union sigval val = {getpid()}; // make sure the receiver knows who send the signal
        if (sigqueue(receiver_pid, signo, val) < 0){
            perror("sigqueue");
            exit(1);
        }
        // if we failed we will increase the wait time so the next time we wait a little longer
        time = time > thresh ? 0 : (time + 0.3);
        // wait for parent signal
        sleep(time);
    }
}

void send_signal_to(pid_t receiver_pid, int signo, void (*handler)(int, siginfo_t *, void *)) {
    // set the signal handler
    set_signal_handler(signo, handler);
    const union sigval val = {getpid()}; // make sure the receiver knows who send the signal
    if (sigqueue(receiver_pid, signo, val) < 0) {
        perror("sigqueue");
        exit(1);
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
#pragma once


// standard include headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <poll.h>
#include <sys/errno.h>
#include <stdbool.h>
#include <syscall.h>
#include <signal.h>
#include <fcntl.h>

// my include header
#include "Types.h"

// define the number of different prime algorithms we will use
#define PRIME_ALGOS 3

// default number of children-processes created
#define CHILD_PROC_DEF_NUM 3

// the actual number of children-processes created
uint ch_proc_thresh;


// define the read and write indexes, for pipe communication
#define READ 0
#define WRITE 1
#define TIMEOUT 2

// function that creates children (different impl, same declaration)
typedef void (*CreateChildren)(int num_of_children, Range* ranges);

// process calls this and waits for all its children
void wait_children(void);

// internal processes behaviour (root and internal process nodes)
void internal_node_behaviour(int argc, char *argv[], CreateChildren create_children);

// fd : file descriptor to perform reading
void print_primes_from_child(int fd);

// function for Async I/O between pipes (WARNING IT DISSAMBLES THE SINGAL HANDLER)
void internal_read_from_child(int fd_array[][2], int number_of_children);

// function to close the sibling pipes besides the current child-index
void close_sibl_pipes(int fd_board[][2], int child_index, int num_of_children);

// signal utilities

// function to send signo to the receiver till he gets it
// set a handler for any signo signal received from the current process
void wait_signal_from(pid_t receiver_pid, int signo, void (*handler)(int, siginfo_t *, void *));

// easy to set signal handler (usign sigact)
void set_signal_handler(int signo, void (*handler)(int, siginfo_t *, void *));

// ignore the signal signo (set the handler SIG_IGN)
void ignore_signal(int signo);

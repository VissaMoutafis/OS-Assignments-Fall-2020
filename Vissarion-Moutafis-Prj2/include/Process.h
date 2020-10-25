#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/poll.h>
#include <sys/errno.h>
#include <stdbool.h>
#include "Types.h"

#define PRIME_ALGOS 3

// default number of children-processes created
#define CHILD_PROC_DEF_NUM 3

// the actual number of children-processes created
uint ch_proc_thresh;

#define READ 0
#define WRITE 1

typedef void (*CreateChildren)(int num_of_children, Range* ranges);

// wait for all the children
void wait_children(void);
// behaviour of internal processes
void internal_node_behaviour(int argc, char *argv[], CreateChildren create_children);
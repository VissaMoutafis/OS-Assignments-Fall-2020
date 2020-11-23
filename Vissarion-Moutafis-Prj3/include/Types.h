#pragma once

// Classic includes
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <errno.h>
#include <time.h>

// for the IPC
#include <fcntl.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <syscall.h>
#include <unistd.h>

// global defines for all processes
#define TOMATO "tomato"
#define ONION "onion"
#define PEPPER "pepper"
#define MUTEX "salad-mutex"
// failure to create
#define ORDER_FAILURE_CRT -1    
// general success code
#define ORDER_SUCCESS 0

// typedef to distinquish the ingredients
typedef sem_t* Ingredients;

// order struct protorype
typedef struct {
    int salad_counter_id;
    void *salad_counter;
} Order;

typedef Order ShmPair;

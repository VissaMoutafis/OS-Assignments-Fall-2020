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
#define SALAD_WORKER "salad-workers-out"
#define WORKING_TABLE "working-table"
// failure to create
#define ORDER_FAILURE_CRT -1    
// general success code
#define ORDER_SUCCESS 0

// typedef to distinquish the ingredients
typedef sem_t* Ingredients;

typedef enum {tomato=0, onion=1, pepper=2} SaladMakerIndex;

typedef struct {
    int salads_per_saladmaker[3];
    int num_of_salads;
} SharedMem;

// order struct protorype
typedef struct {
    int shm_id;
    SharedMem* shmem;
} Order;

typedef Order ShmPair;


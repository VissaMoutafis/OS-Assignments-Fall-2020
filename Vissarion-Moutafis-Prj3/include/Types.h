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
#define WORKING_TABLE "working-table"
#define LOG_MUTEX "salad-log-mutex"

// path to the public log
#define LOG_PATH "./logs/common-log"

// format for time printing
#define USER_TIME_FORMAT "%H:%M:%S"

// failure to create
#define ORDER_FAILURE_CRT -1    
// general success code
#define ORDER_SUCCESS 0

// typedef to distinquish the ingredients
typedef sem_t* Ingredients;

typedef enum {tomato=0, onion=1, pepper=2} SaladMakerIndex;

typedef enum {
    log_code_start = 0,
    log_code_end,
    log_code_stats,
    log_code_cook_start,
    log_code_cook_end,
    log_code_provide_ingr,
    log_code_receive_ingr
} LogCode;

typedef struct {
    int salads_per_saladmaker[3];
    int num_of_salads;
    int num_of_finished;
} SharedMem;

// order struct protorype
typedef struct {
    int shm_id;
    SharedMem* shmem;
} Order;

typedef Order ShmPair;


typedef void *Pointer; //We will use the Pointer notation for Item and/or Key type

typedef int (*Compare)(Pointer item1, Pointer item2); //Function to compare 2 items given by the user

typedef void (*ItemDestructor)(Pointer item); //Function to delete the items since the user allocates the memory for the items all alone

typedef void (*Visit)(Pointer item); //Function to help the user define the way the items are printed

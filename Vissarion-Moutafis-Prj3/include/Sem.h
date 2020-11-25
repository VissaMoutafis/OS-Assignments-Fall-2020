#pragma once

// semaphore related activites
#include "Types.h"

// create the semaphore and return a pointer to it
sem_t *sem_create(const char *name, unsigned int init_value);

// open the semaphore with name "name"
sem_t *sem_retrieve(const char *name);

// dettach the process from the semaphore
void sem_dettach(sem_t *sem);

// dettach the process from the semaphore and unlink it 
void sem_clear(char *name, sem_t *sem);

// P procedure: wait, "decrease the semaphores counter"
void sem_P(sem_t *sem);

int sem_P_nonblock(sem_t *sem);

// V procedure: signal, "increase the semaphore counter"
void sem_V(sem_t *sem);


// debbuging utlity
void sem_print(char *name, sem_t *sem);
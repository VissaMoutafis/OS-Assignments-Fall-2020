#pragma once

// semaphore related activites
#include "Types.h"

// create the shared segment of specified size, return the shmid, < 0 for error 
int shm_create(size_t size);

// get the memory pointer, related to the shmid
void *shm_attach(int shmid);

// mark the segment for destruction after last active process dettaches from it
void shm_destroy(int shmid);

// get the relative info struct for the given shmid
struct shmid_ds shm_read(int shmid);

// dettach the process from the given shmid
void shm_dettach(void *shmem);
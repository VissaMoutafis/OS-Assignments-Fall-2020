#include <stdio.h>
#include "SharedSeg.h"

int shm_create(size_t size) {
    int id = shmget(IPC_PRIVATE, size, 0666);
    if (id < 0) 
        perror("Creating shared segment");

    return id;
}

void *shm_attach(int shmid) {
    assert(shmid > -1);
    void * mem = shmat(shmid, NULL, 0);
    if ( mem == (void*)-1)
        perror("Attaching the shared segment");
    return mem;
}

// mark the shared segment to be destructed after last process is dettached
void shm_destroy(int shmid) {
    assert(shmid > -1);
    int err = shmctl(shmid, IPC_RMID, 0);
    if (err == -1) 
        perror ("Removing shared segment");
}

struct shmid_ds shm_read(int shmid) {
    assert(shmid > -1);
    static struct shmid_ds shm;
    memset(&shm, 0, sizeof(shm));
    int err = shmctl(shmid, IPC_STAT, &shm);
    if (err == -1) perror("Reading shared segment");

    return shm;
}

void shm_dettach(void *shmem) {
    assert(shmem);
    int err = shmdt(shmem);
    char b[100];
    sprintf(b, "Process: %d, Dettaching from the shared segment", getpid());
    if (err == -1)
        perror(b);
}
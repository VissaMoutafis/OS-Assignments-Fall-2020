#include <stdio.h>
#include "Sem.h"

// create a semaphore
sem_t *sem_create(const char *name, unsigned int init_value) {
    sem_t *sem = sem_open(name, O_CREAT|O_EXCL, S_IRUSR|S_IWUSR, init_value);
    if (sem == SEM_FAILED) {
        char buf[500];
        sprintf(buf, "Semaphore %s", name);
        perror(buf);
        return  sem_retrieve(name);
    }

    return sem;
}

sem_t* sem_retrieve(const char *name) {
    sem_t *sem = sem_open(name, 0);
    if (sem == SEM_FAILED) {
        char buf[500];
        sprintf(buf, "Semaphore %s can't be linked", name);
        perror(buf);
    }

    return sem;
}

void sem_dettach(sem_t *sem) {
    assert(sem != SEM_FAILED);
    sem_close(sem);
}

void sem_clear(char *name, sem_t *sem) {
    assert(sem != SEM_FAILED);
    sem_close(sem);
    int err = sem_unlink(name);

    if (err == ENOENT) {
        char buf[500];
        sprintf(buf, "Semaphore %s doesn't exist", name);
        perror(buf);
    }
}

void sem_P(sem_t *sem) {
    assert(sem != SEM_FAILED);
    sem_wait(sem);
}

int sem_P_nonblock(sem_t *sem) {
    assert(sem != SEM_FAILED);
    return sem_trywait(sem);
}

void sem_V(sem_t *sem) {
    assert(sem != SEM_FAILED);
    sem_post(sem);
}

void sem_print(char *name, sem_t *sem) {
    int val;
    sem_getvalue(sem, &val);
    printf("Semaphore '%s' val = %d\n", name, val);
}
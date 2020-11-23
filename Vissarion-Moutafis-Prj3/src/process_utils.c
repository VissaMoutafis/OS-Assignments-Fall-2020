#include "ProcessUtils.h"

void wait_children(void) {
    int pid, status;
    while ((pid = wait(&status)) != -1) {
        // printf("Child %d exited with status %d.\n", pid, status);
    }
    // printf("Parent %d exited\n", getpid());
}
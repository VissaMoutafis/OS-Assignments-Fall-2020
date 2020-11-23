#pragma once

// Classic includes
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

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

// typedef to distinquish the ingredients
typedef enum { tomato = 0, onion, pepper } Ingredients;
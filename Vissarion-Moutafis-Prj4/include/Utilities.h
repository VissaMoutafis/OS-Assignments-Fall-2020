#pragma once

#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "FileManipulation.h"


// return the mode, in failure return something < 0
mode_t path_get_mode(char *path);

// return 1 if the path is a directory, else 0
int is_dir(char *path);

// create a dir with perms 0777
DIR *create_dir(char *path);

// return 1 if files differ, 0 if they don't
int files_diff(char *in_path, char *out_path);

// return 1 if the path is symbolic link, else 0
int is_sym(char *path);

// function that decreases the link count of the specific element's inode
void delete_element(char *path);

int get_hardlinks(char *path);

// small procedure that takes as input the __opened__ fd's
// of one input and one output file and copies the containings of
// the first to the second. Returns 1 in success, otherwise 0
int copy_file(int in_fd, int out_fd, int BUFFSIZE);

// check for the files in target that are deleted from source and delete them from target
int check_deleted(char *src_path, char *trg_path);

// check if the target path is inside the src path and we have an infinite loop
// return SUCC if it detects a cycle, otherwise return FAIL 
int detect_cycle(char *src_path, char *trg_path);
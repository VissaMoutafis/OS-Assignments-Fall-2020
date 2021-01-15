#pragma once

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "HT.h"

// Declaration of various needed variables among uilities and file manip files.

// total bytes copied
u_int64_t bytes_copied;
// total number of new elements copied
u_int32_t items_copied;
// total number of elements checked for copying
u_int32_t items_detected;
// total time the whole copy process lasted
double total_time;
// flag that is used to check if a directory changed after recursive calls to
// its contents
short dir_changed;


#define CP_BUFFSIZE 10

#define FILE_CP_SUCC 1
#define FILE_CP_FAIL 0

#define DIR_CP_SUCC 1
#define DIR_CP_FAIL 0

#define SUCC 1
#define FAIL 0

#define ACCESS_RIGHTS_MASK 0x1ff

// set to 0 if you don't want to consider symlinks, else set it to 1
int manage_links;

// function that copies a file, given in in_path, using a batch of BUFFSIZE and 
// creates the new file with permissions PERMS and if the file exists then it truncates it
// returns 1 in success and 0 in failure
int clean_copy_file(char *in_path, char *out_path, int BUFFSIZE, char *out_root_path);

// function that can copy all kinds of notions in the in path (dirs are copied recursively)
int copy_element(char *in_path, char *out_path, char *out_root_path);



// variables used for statistics printing 
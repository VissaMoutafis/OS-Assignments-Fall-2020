/*
** Implemented by Vissarion Moutafis
*/

#pragma once

#include "Types.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

char *make_str(FILE **_stream_);                            // get a line from the stream as a string
size_t fget_lines(char *filename);                          // get the #lines of file with name 'filename'
char **parse_line(char *data_str, int *columns, char *sep); // classic line parser (mush alike python's str.split)
bool is_numeric(char *str);                                 // check if the string is numeric
char *num_to_str(int num);                                  // int -> string
int get_len_of_int(int num);                                // get the number of digits an integer has
void print_error(char *msg);                                // print error msg in stderr


// Argument parsing utilities
int find_arg_index(char *arg_list[], int argc, char *arg);
bool check_args(int argc, char* argv[], char* proper_args[], int proper_args_size, char *num_args[], int num_args_size);
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

// Argument Parsing Tool:
// pass the argc, argv from main function call
// input: the string array the result will be returned (size: input_size)
// allowed args of size arg_num: ALL (valued & unvalued) the arguments the user wants to use
// valued args of size val_args_num: the arguments that take value (i.e. -i "input")
// usage_format : message like "program -c value -i value -o value" for the error printing
typedef struct
{
    char **args;
    int num;
} Arguments;

void args_parser(int argc, char **argv, char ***input, int *input_size, Arguments allowed_arguments, Arguments valued_args, Arguments must_args, char *usage_format);

#pragma once

#include "Types.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

char *make_str(FILE **_stream_);
size_t fget_lines(char *filename);
char **parse_line(char *data_str, int *columns, char* sep);
bool is_numeric(char* str);

// arg parsing
void args_parser(int argc, char **argv, char ***input, int *input_size);
void parse_cnfg(char* filename); // parse config file
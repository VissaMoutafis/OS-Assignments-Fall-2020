#pragma once

#include "Types.h"
#include "ParsingUtils.h"
#include <stdio.h>
#include <string.h>


// ###################### Caller can change this to apply to his constraints ############### //
// all the possible commands for the tty API of the app
int pos_cmds_len;

// for format checking
char *allowed_formats[10];

// for help printing
char *possible_commands[10];
    

// Methods to print messages
void help(void);
char *get_input(void);
bool check_format(char *expr, int *expr_index);
char **parse_expression(char* expr);
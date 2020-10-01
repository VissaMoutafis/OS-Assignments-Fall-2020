#pragma once

#include "Types.h"
#include "ParsingUtils.h"
#include <stdio.h>
#include <string.h>


// ###################### Caller can change this to apply to his constraints ############### //
// all the possible commands for the tty API of the app
int pos_cmds_len = 10;

// for format checking
char *allowed_formats[10] = {
    "i",
    "l",
    "d",
    "n",
    "t",
    "a",
    "m",
    "c",
    "p",
    "exit"
    };

// for help printing
char *possible_commands[10] = {
    "i(nsert) [studentid, lastname, firstname, zip, year, gpa]",
    "l(ook-up) [studentid]",
    "d(elete) [studentid]",
    "n(umber) [year]",
    "t(op) num [year]",
    "a(verage) [year]",
    "m(inimum) [year]",
    "c(ount)",
    "p(ostal code) [rank]",
    "exit"};
    

// Methods to print messages
void help(void);
char *get_input(void);
bool check_format(char *cmd);
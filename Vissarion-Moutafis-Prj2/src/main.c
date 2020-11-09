/*
** Implemented by Vissarion Moutafis
*/

#include "Types.h"
#include "Process.h"
#include "ParsingUtils.h"
#include <assert.h>

typedef enum {check_num=0, check_type}ArgChecks;

// check for proper number of args and perform typecheck (based on check valeu)
static void check_args_specific(int argc, char* argv[], ArgChecks check, int desired_args, char* numerical_argv[], int n_args_num) {
    switch (check)
    {
    case check_num:
        if (argc < 2*desired_args+1) {
            perror("Number of args is wrong");
            exit(1);
        }
    break;
    
    case check_type:
        for (int i = 0; i < n_args_num; i++) {
            if (!is_numeric(numerical_argv[i])) {
                perror("Wrong input in numerical arg");
                exit(1);
            }
        }
    break;
    }
}

static void reform_args(char *reformed_argv[], int argc, char* argv[]) {
    // very specific function
    
    assert(reformed_argv);

    // first check for proper ammount of arguments
    check_args_specific(argc, argv, check_num, 3, NULL, 0);

    for(int i = 0; i < (argc-1)/2; i++)
        reformed_argv[i] = NULL;

    // Now lets reform them
    for (int i = 1; i < argc; i+=2) {
        
        switch (argv[i][1]) {
        case 'l':
            if (reformed_argv[0]) {
                perror("Can't set the same argument the same time");
                exit(1);
            }
            reformed_argv[0] = argv[i+1];     
        break;
        
        case 'u':
            if (reformed_argv[1]) {
                perror("Can't set the same argument the same time");
                exit(1);
            }
            reformed_argv[1] = argv[i+1];
        break;
        
        case 'w':
            if (reformed_argv[2]) {
                perror("Can't set the same argument the same time");
                exit(1);
            }
            reformed_argv[2] = argv[i+1];
        break;

        default:
            perror("wrong argument!");
            exit(1);
            break;
        }
    }
    // check if every argument is numerical
    check_args_specific(argc, reformed_argv, check_type, 0, reformed_argv, 3);
}

int main(int argc, char* argv[]) {
    char *reformed_argv[(argc-1)/2];

    reform_args(reformed_argv, argc, argv);

    // Now it's time to execute the root program

    if (execl("./root", "./root", "-l", reformed_argv[0], "-u", reformed_argv[1], "-w", reformed_argv[2], (char*)0) < 1) {
        perror("execl in main function calling root");
        exit(1);
    }
}
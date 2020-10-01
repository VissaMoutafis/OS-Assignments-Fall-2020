#include "ParsingUtils.h"

void error_handle(char **a) {
    // wrong input
    // first free the memory
   if (a) {     
       for (int j = 0; j < 3; j++)
            if (a[j] != NULL)
                free(a[j]);

        free(a);
    }

    printf("Error in the input format.\n \
        Usage:    ~$ program_name -i \"input file\" -c \"config file\" \
        \n------------------------------------------------------------------------------------------\n");
}
void error_false_arg(void) {
    printf("\n- - - Error: Passed False Argument.");
}

void error_too_many_args(void) {
    printf("\n- - - Error: Too Many Arguments.\n");
}

// The function must return either true or false for successful
// parsing or not.
// Also we will pass a pointer to a string-array so that
// the user gets the input values in some way
bool args_parser(int argc, char **argv, char ***input, int *input_size) {
    
    // Basic constraints :
    // 1. the input values must be at most 4 + 1(the program name) values, 7 in total
    // 2. the 2 program flags are : -i "input file" -c "config file"

    if (argc <= 5 && input != NULL) {
        // some basic error handling
        // set the proper ammount of memory and initiaze it with null values
        *input = calloc(2, sizeof(char *));
        *input_size = (argc - 1) / 2;     //set the input size so that the caller knows how many flags there are
        for (int i = 1; i < argc; i += 2) // we begin from the second index since the first one is the program's name
        {
            // set the flag value
            char *flag = argv[i];

            // get a copy of the flag's value
            char *value = calloc(strlen(argv[i + 1]) + 1, sizeof(char));
            strcpy(value, argv[i + 1]);

            // set the proper field equal to the value
            if (strcmp(flag, "-i") == 0)
                (*input)[0] = value;
            else if (strcmp(flag, "-c") == 0)
                (*input)[1] = value;
            else
            {
                error_false_arg();
                error_handle(*input);

                //return false to the caller
                return false;
            }
        }
        //everything went good return true
        return true;
    }
    error_too_many_args();
    error_handle(NULL);    
    return false;
}

// test main
// int main(int argc, char **argv) {
//     int input_size;
//     char** input;
//     bool s = args_parser(argc, argv, &input, &input_size);
//     if (input_size == 0) printf("No Args\n");
//     for (int i=0; s && i<input_size; ++i) {
//         printf("Argument #%d : %s\n", i, input[i]);
//     }
//     return 0;
// }
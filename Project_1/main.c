
#include "ManageStudents.h"

int main(int argc, char **argv) {
    // the main logic of the menu is the following:
    // 1. get the input
    // 2. parse it
    // 3. give it to the runelection struct to make something out of it

    char **arguments;
    int size;

    args_parser(argc, argv, &arguments, &size, (char *[2]){"-i", "-c"}); // Something must be done for the configurations
    if (size == 2) {
        cnfg = true;
        parse_cnfg(arguments[1]);
    } else {
        cnfg = false;
    }

    // turn the is_end pointer to false
    is_end = false;
    
    // Initiaze the struct
    ManageStudents manager = mngstd_create(student_compare, student_destructor, student_hash, arguments[0]);
   

    while (manager && !is_end) {
        // Get the tty expression
        char *expr = get_input();
        // Parse the expression
        char **parsed_cmd = parse_expression(expr); // parse the given experssion (format:~$ command value)
        
        // make clean what we are talking about
        char *command = parsed_cmd[0];
        char *value = parsed_cmd[1];
        int expr_index;

        // check if the expression is right and 
        if (check_format(command, &expr_index) == true)
            mngstd_run(manager, expr_index, value);
            // printf("The program will run {command: %s, value: %s}\n", parsed_cmd[0], parsed_cmd[1]);
        else
            help();

        if(expr)free(expr);
        if(parsed_cmd[0]) free(parsed_cmd[0]);
        if(parsed_cmd[1]) free(parsed_cmd[1]);
        if(parsed_cmd) free(parsed_cmd);
    }

    // de-allocate the memory
    mngstd_destroy(manager);

    for(int i = 0; i < size; ++i) 
        free(arguments[i]);

    free(arguments);
}
#include "ParsingUtils.h"

// returns the index of the arg or -1 if fails
int find_arg_index(char *arg_list[], int argc, char *arg) {
    for (int i = 0; i < argc; i++) {
        if (strcmp(arg_list[i], arg) == 0)
            return i;
    }

    return -1;
}

bool check_args(int argc, char* argv[], char* proper_args[], int proper_args_size, char *num_args[], int num_args_size) {
    if (argc < 1+2*proper_args_size) {
        print_error("Wrong number of arguments");
        return false;
    }
    int vis[proper_args_size];
    for (int i = 0; i < proper_args_size; i++) 
        vis[i] = false;
    
    for (int i = 1; i < argc; i += 2) {
        int num_id = find_arg_index(num_args, num_args_size, argv[i]);
        int proper_id = find_arg_index(proper_args, proper_args_size, argv[i]);
        
        // if the arg is not in the proper list then we have an error
        if (proper_id == -1) {
            print_error("False/Unkown argument flag.");
            return false;
        } else if (num_id != -1) {
            char *value = argv[i+1];
            if (!is_numeric(value)) {
                print_error("False input in numeric argument flag.");
                return false;
            }
        } else if (find_arg_index(proper_args, proper_args_size, argv[i+1]) != -1) {
            print_error("Missing argument value.");
            return false;
        }
        vis[proper_id] = true;
    }
    for (int i = 0; i < proper_args_size; i++)
        if (vis[i] != true) { 
            print_error("Wrong Arguments inserted");
            return false;
        }
    return true;
}
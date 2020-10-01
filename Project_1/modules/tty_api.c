#include "TTY.h"

void print_tty(void) {
    // NOTE: Maybe later add some color
    printf("<mngstd> ~$ ");
}

void help(void) {
    printf("Usage for the tty API:\n");
    for (int i = 0; i < pos_cmds_len; i++) {
        printf("\t%s\n", possible_commands[i]);
    }
}

char *get_input(void) {
    char *in;

    print_tty();
    in = make_str(&stdin);
    printf("\n");
    return in;
}

bool check_format(char* cmd) {
    // linear search for the format
    for (int i = 0; i < pos_cmds_len; i++)
        if (strcmp(cmd, allowed_formats[i]) == 0)
            return true;
    
    // if not found return error
    return false;
}

// int main() {
//     printf("%s\n",check_format(get_input()) ? "Right" : "Wrong Format");
//     return 0;
// }
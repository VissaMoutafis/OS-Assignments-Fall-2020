#include "TTY.h"

char *make_str(FILE **_stream_) {
    /*
    ** A utility to transform a stream input to a string.
    ** there is no need to enter a string length
    ** WARNING: It returns the whole line and only the line. Parse it carefully.
    */
   
    // First we will get the line
    unsigned int len = 1;
    char *str = calloc(1, sizeof(char));
    unsigned int c;

    while ((c = fgetc(*_stream_)) != EOF && c != '\n') {
        len++;
        char *new_str = calloc(len, sizeof(char));
        strcpy(new_str, str);
        new_str[len - 2] = c;
        free(str);
        str = new_str;
    }

    if (strlen(str) == 0 && c == EOF) { 
        // if we are at the end of the file and 
        // the str is empty just free the 1 byte and return EOF
        free(str);
        return NULL;
    }

    return str; // return the str
}

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

int main() {
    printf("%s\n",check_format(get_input()) ? "Right" : "Wrong Format");
    return 0;
}
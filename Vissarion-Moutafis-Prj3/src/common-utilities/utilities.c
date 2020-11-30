/*
** Usefull utilities like string parsers and file lines to string converters
**  Written by Vissarion Moutafis sdi1800119
*/
#include "ParsingUtils.h"
#include "Sem.h"

void get_time_str(char *buffer, int size) {
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, size, USER_TIME_FORMAT, timeinfo);
}

struct tm *get_time(void) {
    time_t rawtime;
    time(&rawtime);
    return localtime(&rawtime);
}

void print_error(char *msg) {
    fprintf(stderr, "Error: %s\n", msg);
}

char **parse_line(char *data_str, int *columns, char* sep) {
    // Initialize the columns counter
    *columns = 0;
    
    char **data = NULL;

    // split the str to all space-separated substring
    char *tok = strtok(data_str, sep);

    while (tok != NULL) {
        // for all the tokens we just keep on going to parse the data correclty

        // increase the counter
        (*columns)++;
        // //resize the array
        char **new_data = calloc(*columns, sizeof(char *));

        if (data != NULL) {
            memcpy(new_data, data, ((*columns) - 1) * sizeof(char *));
            free(data);
        }

        // re-assign the old data table to the new one
        data = new_data;

        // create a copy of the token
        data[(*columns) - 1] = malloc((strlen(tok) + 1) * sizeof(char));
        // .. and add i to the data table
        strcpy(data[(*columns) - 1], tok);

        tok = strtok(NULL, sep);
    }

    return data;
}

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

// classic trick to count the number of lines in a file
size_t fget_lines(char *filename) {
    FILE *fin = fopen(filename, "r"); // open the file for reading
    if (!fin) {
        printf("The file you entered does not exist!\n");
        exit(EXIT_FAILURE);
    }
    size_t count = 0;
    unsigned char c;
    while ((c = fgetc(fin)) != EOF && !feof(fin))
        count = (c == '\n') ? count + 1 : count;

    // close the file properly
    fclose(fin);

    return count;
}

bool is_numeric(char* str) {
    for (int i = 0; str[i]; i++)
        if (!(str[i] >= '0' && str[i] <= '9'))
            return false;
    
    return true;
}


char* num_to_str(int num) {
    if (num == 0) return "0";

    char* str = calloc(1, sizeof(char));
    int len = 1;
    while (num) {
        char dig = num%10 + '0';
        num /= 10;

        char* new_str = calloc(++len, sizeof(char));
        strcpy(new_str+1, str);
        new_str[0] = dig;
        free(str);
        str = new_str;
    }

    return str;
}

int get_len_of_int(int num) {
    // this function will find the length of integers given
    int cnt = 0;
    while (num) {
        cnt++;
        num /= 10;
    }

    return cnt;
}

int get_int_in(int l, int h) {
    if (l == h) h++;

    int n = l - 1;
    do {
        n = rand() % h;
    } while (n < l || n > h);

    return n;
}

void print_log(LogCode code, FILE *file, char* msg, sem_t *log_mutex) {
    // get the current time
    char time_buf[80];
    get_time_str(time_buf, 80);

    // we must write atomicaly so we call the mutex wait call
    if (log_mutex)
        sem_P(log_mutex);
    // get to the end of file (append only protocol)
    fseek(file, 0, SEEK_END); 
    // write the message
    fprintf(file, "=%d,%s= %s \n", code, time_buf, msg); 
    // flush the file so there is no buffering
    fflush(file);
    if (log_mutex)
        sem_V(log_mutex);
    // end of the write (signal the mutex semaphore)
}
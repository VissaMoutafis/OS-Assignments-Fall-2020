/*
** Usefull utilities like string parsers and file lines to string converters
**  Written by Vissarion Moutafis sdi1800119
*/
#include <sys/time.h>
#include <time.h>
#include "ParsingUtils.h"
#include "Sem.h"

// a typedefed enum type needed for the string-parsing-to-MyTime struct-function
typedef enum { hours = 0, minutes = 1, seconds = 2, milliseconds = 3 } TimePart;

void get_time_str(char *buffer, int size) {
    char usec_buf[20];
    struct tm *timeinfo;
    struct timeval tv;

    gettimeofday(&tv, NULL);

    timeinfo = localtime(&tv.tv_sec);
    strftime(buffer, size, USER_TIME_FORMAT, timeinfo);
    strcat(buffer, ":");
    int m_sec = (int)tv.tv_usec / 10000;
    if (m_sec < 10) m_sec *= 10;
    sprintf(usec_buf, "%d", m_sec);
    strcat(buffer, usec_buf);
}

void MyTime_copy(MyTime *dest, MyTime src) {
    assert(dest);
    dest->hour = src.hour;
    dest->min = src.min;
    dest->sec = src.sec;
    dest->millisec = src.millisec;
}

void MyTime_get_time(MyTime *my_time) {
    assert(my_time);
    time_t rawtime;
    time(&rawtime);
    struct tm *timeinfo;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    timeinfo = localtime(&rawtime);
    my_time->hour = timeinfo->tm_hour;
    my_time->min = timeinfo->tm_min;
    my_time->sec = timeinfo->tm_sec;
    my_time->millisec = (int)tv.tv_usec/10000; // 2 digs accuracy
}

// return in format HH:MM:SS:ss
void MyTime_time_to_str(MyTime *my_time, char *buffer, int buf_size) {
    assert(buf_size > 11); // min buffer size
    assert(my_time);
    
    char usec_buf[10]; // buffer to write milliseconds
    // write from the struct tm* first
    struct tm t;
    t.tm_hour = my_time->hour;
    t.tm_min = my_time->min;
    t.tm_sec = my_time->sec;
    strftime(buffer, buf_size, "%H:%M:%S", &t);
    strcat(buffer, ":");
    // concatenate the 2 buffers
    sprintf(usec_buf, "%d", my_time->millisec);
    strcat(buffer, usec_buf);
}

static void set_time_part(MyTime *my_time, int new_val, TimePart time_part) {
    switch (time_part) {
    case hours:
        MyTime_set_hours(my_time, new_val);
    break;

    case minutes:
        MyTime_set_minutes(my_time, new_val);
    break;

    case seconds:
        MyTime_set_seconds(my_time, new_val);
    break;
    
    case milliseconds:
        MyTime_set_milliseconds(my_time, new_val);
    break;
    default:
        fprintf(stderr, "set_time_part: false value in time_part variable\n");
        break;
    }
}

// pass in format HH:MM:SS:ss
void MyTime_str_to_time(MyTime *my_time, char *src_buffer, int src_buf_size) {
    assert(my_time);
    assert(strlen(src_buffer) == 11);
    char b[3];
    int b_id = 0;
    memset(b, 0, 3);
    TimePart  t = hours;
    // fprintf(stderr, "============MyTime_str_to_time: Not yet needed, Should be implemented.\n");
    for (int i = 0; i <= src_buf_size; i++) {
        if (src_buffer[i] == ':' || i == src_buf_size) {
            // add the time part to the proper variable
            set_time_part(my_time, atoi(b), t);
            t = (t+1)%4;
            memset(b, 0, 3);
            b_id = 0;
        } else if (isdigit(src_buffer[i])){
            b[b_id] = src_buffer[i];
            b_id++;
        }
    }
}

int MyTime_get_hours(MyTime *my_time) {
    assert(my_time);
    return my_time->hour;
}
int MyTime_get_minutes(MyTime *my_time) {
    assert(my_time);
    return my_time->min;
}
int MyTime_get_seconds(MyTime *my_time) {
    assert(my_time);
    return my_time->sec;
}
int MyTime_get_milliseconds(MyTime *my_time) {
    assert(my_time);
    return my_time->millisec;
}
void MyTime_set_hours(MyTime *my_time, int new_val) {
    assert(my_time);
    my_time->hour = new_val;
}
void MyTime_set_minutes(MyTime *my_time, int new_val) {
    assert(my_time);
    my_time->min = new_val;
}
void MyTime_set_seconds(MyTime *my_time, int new_val) {
    assert(my_time);
    my_time->sec = new_val;
}
void MyTime_set_milliseconds(MyTime *my_time, int new_val) {
    assert(my_time);
    my_time->millisec = new_val;
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
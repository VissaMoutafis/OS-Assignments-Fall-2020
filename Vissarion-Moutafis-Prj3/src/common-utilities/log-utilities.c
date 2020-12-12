#include <sys/time.h>
#include <time.h>
#include "ParsingUtils.h"
#include "Sem.h"

void print_log(LogCode code, FILE *file, char* process_name, char *msg, sem_t *log_mutex) {
    // get the current time
    char time_buf[80];
    get_time_str(time_buf, 80);

    // we must write atomicaly so we call the mutex wait call
    if (log_mutex) sem_P(log_mutex);
    // get to the end of file (append only protocol)
    fseek(file, 0, SEEK_END);
    // write the message
    fprintf(file, "[%d] [%s] [%d] [%s] [%s]\n", code, time_buf, getpid(), process_name, msg);
    // flush the file so there is no buffering
    fflush(file);
    if (log_mutex) sem_V(log_mutex);
    // end of the write (signal the mutex semaphore)
}

static bool is_bracket(char ch) {
    return ch == '[' || ch == ']';
}

static LogCode get_log_code(char *log_line){
    // General logline format [code] [timestamp] [pid] [proc name] [msg]
    char buf[10];
    memset(buf, 0, 10);
    int brackets = 0;
    int buf_id = 0;
    for (int i = 0; brackets < 2 && i < strlen(log_line); i++) {
        if (is_bracket(log_line[i]))
            brackets++;
        else {
            assert(buf_id < 10);
            buf[buf_id] = log_line[i];
        }
    } 

    return atoi(buf);
}

static MyTime get_timestamp(char *log_line) {
    // General logline format [code] [timestamp] [pid] [proc name] [msg]
    int i = 0;
    int brackets = 0;
    while (brackets < 3){
        if (is_bracket(log_line[i]))
            brackets ++;
        i++;
    } 

    assert(is_bracket(log_line[i-1]));
    // get the time string
    char time_buf[12];
    memset(time_buf, 0, 12);
    strncpy(time_buf, &log_line[i], 11);

    // get the time and return it in a struct
    MyTime t;
    MyTime_str_to_time(&t, time_buf, 12);

    return t;
}
// write the user of the logline to the usr_buf
static void get_usr(char* log_line, char *usr_buf, int usr_buf_size) {
    // General logline format [code] [timestamp] [pid] [proc name] [msg]
    int i = 0;
    int brackets = 0;
    while (brackets < 7) {
        if (is_bracket(log_line[i])) brackets++;
        i++;
    }

    assert(is_bracket(log_line[i-1]));

    char usr[100];
    memset(usr, 0, 100);
    int usr_i = 0;
    int j;
    // copy it to the usr buffer
    for (j = i; log_line[i] && !is_bracket(log_line[j]); j++) {
        usr[usr_i] = log_line[j];
        usr_i++;
    }

    //debbuging line
    assert(is_bracket(log_line[j]));
    // copy the usr buffer to the usr_buf string and return
    if (strlen(usr) >= usr_buf_size) {
        fprintf(stderr, "get_usr in log-utilities.c: The given buf is to small to hold the user");
        strcpy(usr_buf, "Unknown");
    } else {
        strcpy(usr_buf, usr);
    }
}

static int get_proc_pid(char *log_line) {
    // General logline format [code] [timestamp] [pid] [proc name] [msg]
    int i = 0;
    int brackets = 0;
    while (brackets < 5) {
        if (is_bracket(log_line[i])) brackets++;
        i++;
    }

    assert(is_bracket(log_line[i-1]));

    char pid[100];
    memset(pid, 0, 100);
    int pid_i = 0;
    while (!is_bracket(log_line[i])) {
        pid[pid_i] = log_line[i];
        pid_i++;
        i++;
    }

    return atoi(pid);
}

void get_workers_pid(char *logfiles[], int pids[], int size) {
    // we want to read the first line in each of these files since there lies the pid of each one of them
    for (int fid = 0; fid < size; fid++) {
        FILE *f = fopen(logfiles[fid], "r");
        assert(f);
        char *first_line = make_str(&f);
        pids[fid] = get_proc_pid(first_line);
        free(first_line);
        fclose(f);
    }
}

// Create a board of MyTimeInterval structs for each one of the specified users in the user board.
// We consider the log file with name log_name
// Acquire start/end timestamps by the lines that has start_code/end_code as LogCode  
MyTimeInterval** get_time_intervals_from_log(char *log_name, LogCode start_code, LogCode end_code, char *usr_list[], int usr_list_size, int interval_counters[]) {
    // get the #lines of the logfile
    size_t lines = fget_lines(log_name);
    // open the file for reading
    FILE *logfile = fopen(log_name, "r");

    // that's the interval board we will return
    MyTimeInterval **time_intervals = calloc(usr_list_size, sizeof(*time_intervals));
    // number of intervals for each user, till this point
    int usr_interval_counters[usr_list_size];
    // check for starting or ending intervals
    bool starting_interval[usr_list_size];

    for (int i = 0; i < usr_list_size; i++) {
        usr_interval_counters[i] = 0;
        starting_interval[i] = true;
    }

    for (int line = 0; line < lines; line++) {
        // for each line, do:
        char *line_buf = make_str(&logfile);
        // get the log code
        LogCode code = get_log_code(line_buf);

        // if its a line that we care to parse:
        if (code == start_code || code == end_code) {
            // get the log time
            // the time format is "HH:MM:SS:ss" => 12 characters needed
            MyTime t;
            t = get_timestamp(line_buf);
            // get the user that wrote the log line
            char usr[50];
            memset(usr, 0, 50);
            get_usr(line_buf, usr, 50);
            int usr_id;
            // check the user id
            for (usr_id = 0; usr_id < usr_list_size; usr_id++)
                if (strcmp(usr_list[usr_id], usr) == 0)
                    // if we have a user that we care about 
                    break;
            
            //check if the user is in the given user list
            if (usr_id < usr_list_size) {
                if (starting_interval[usr_id]) {
                    starting_interval[usr_id] = false;                                  // the next timestamp is an ending one 

                    usr_interval_counters[usr_id] += 1;                                 // increase the counter
                    int c = usr_interval_counters[usr_id];
                    MyTimeInterval *old_intervals = time_intervals[usr_id];             // save the previous intervals
                    MyTimeInterval *new_intervals = calloc(c, sizeof(MyTimeInterval));
                    
                    // if the previous array had intervals inside then copy them
                    if (c-1) memcpy(new_intervals, old_intervals, (c-1)*sizeof(MyTimeInterval));

                    // add the new interval
                    MyTime_copy(&new_intervals[c-1].start, t);
                    time_intervals[usr_id] = new_intervals;
                    
                    // free the old interval board
                    free(old_intervals);

                } else {
                    starting_interval[usr_id] = true;                                   // the next timestamp is a starting one
                    int c = usr_interval_counters[usr_id];
                    MyTime_copy(&time_intervals[usr_id][c-1].end, t);
                }
            }
                
        }
        free(line_buf);
    }
    fclose(logfile);
    memcpy(interval_counters, usr_interval_counters, usr_list_size*sizeof(int));
    return time_intervals;
}

// int main(void) {
//     char *path ="../../logs/common-log";
//     char *usr[] = {"Saladmaker1", "Saladmaker2", "Saladmaker3"};
//     int interval_counters[3];
//     MyTimeInterval **intervals = get_time_intervals_from_log(path, log_code_cook_start, log_code_cook_end, usr, 3, interval_counters);  

//     for (int i = 0; i < 3; i ++) {
//         printf("Time intervals for %s:\n", usr[i]);
//         for (int j = 0; j < interval_counters[i]; j++) {
//             printf("[%d:%d:%d:%d - %d:%d:%d:%d] ", intervals[i][j].start.hour,
//                    intervals[i][j].start.min, intervals[i][j].start.sec,
//                    intervals[i][j].start.millisec, intervals[i][j].end.hour,
//                    intervals[i][j].end.min, intervals[i][j].end.sec,
//                    intervals[i][j].end.millisec);
//         }
//         printf("\n");
//     }

// }
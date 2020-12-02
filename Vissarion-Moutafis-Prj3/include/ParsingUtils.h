/*
** Implemented by Vissarion Moutafis
*/

#pragma once

#include "Types.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>

typedef struct {
    int hour;
    int min;
    int sec;
    int millisec;
} MyTime; // self made time in the form of HH:MM:SS:ss

typedef struct {
    MyTime start;
    MyTime end;
} MyTimeInterval;



char *make_str(FILE **_stream_);                            // get a line from the stream as a string
size_t fget_lines(char *filename);                          // get the #lines of file with name 'filename'
char **parse_line(char *data_str, int *columns, char *sep); // classic line parser (mush alike python's str.split)
bool is_numeric(char *str);                                 // check if the string is numeric
char *num_to_str(int num);                                  // int -> string
int get_len_of_int(int num);                                // get the number of digits an integer has
void print_error(char *msg);                                // print error msg in stderr
void get_time_str(char *buffer, int size);
int get_int_in(int l, int h);


// time utils

// get the time to the pointer my_time
void MyTime_get_time(MyTime *my_time);
// get hours
int MyTime_get_hours(MyTime *my_time);
// get minutes
int MyTime_get_minutes(MyTime *my_time);
// get seconds
int MyTime_get_seconds(MyTime *my_time);
// get milliseconds
int MyTime_get_milliseconds(MyTime *my_time);
// set hours of my_time to new_val
void MyTime_set_hours(MyTime *my_time, int new_val);
// set minutes of my_time to new_val
void MyTime_set_minutes(MyTime *my_time, int new_val);
// set seconds of my_time to new_val
void MyTime_set_seconds(MyTime *my_time, int new_val);
// set milliseconds of my_time to new_val
void MyTime_set_milliseconds(MyTime *my_time, int new_val);
// after this call, the buffer will have the time specified in my_time 
// in format "HH:MM:SS:ss" (buffer must be of len > 11)
void MyTime_time_to_str(MyTime *my_time, char *buffer, int buf_size);
// parse the given buffer and put everything in the my_time struct
void MyTime_str_to_time(MyTime *my_time, char *src_buffer, int src_buf_size);
// copy the src time struct to the dest one
void MyTime_copy(MyTime *dest, MyTime src);
// compare 2 time intervals
// return >0 if t1 > t2, 0 if t1 == t2, <0 otherwise
int MyTime_compare(MyTime *t1, MyTime *t2);

// Argument parsing utilities
int find_arg_index(char *arg_list[], int argc, char *arg);
bool check_args(int argc, char* argv[], char* proper_args[], int proper_args_size, char *num_args[], int num_args_size);

// logging utilities

// write a log message to the specified file 
// (if the log_mutex is not null then write it atomically)
void print_log(LogCode code, FILE *file, char *msg, sem_t *log_mutex);
MyTimeInterval **get_time_intervals_from_log(char *log_name, LogCode start_code,
                                             LogCode end_code, char *usr_list[],
                                             int usr_list_size,
                                             int interval_counters[]);
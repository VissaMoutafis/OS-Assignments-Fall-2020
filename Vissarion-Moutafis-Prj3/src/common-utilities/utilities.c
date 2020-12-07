/*
** Usefull utilities like string parsers and file lines to string converters
**  Written by Vissarion Moutafis sdi1800119
*/
#include <sys/time.h>
#include <time.h>
#include "ParsingUtils.h"
#include "Sem.h"
#include "Stack.h"
#include "PQ.h"
#include <assert.h>

typedef struct {
    MyTimeInterval *interval;
    int overlaps;
} *IntervalPair;


void *make_interval(MyTime start, MyTime end) {
    MyTimeInterval *i = malloc(sizeof(*i));
    MyTime_copy(&i->start, start);
    MyTime_copy(&i->end, end);

    return (void *)i;
}
void destroy_interval(void *i) {
    free(i);
}
int compare_intervals(void *a, void *b) {
    MyTimeInterval *i1, *i2;
    i1 = (MyTimeInterval *)a;
    i2 = (MyTimeInterval *)b;

    int s = MyTime_compare(&i1->start, &i2->start);
    int e = MyTime_compare(&i1->end, &i2->end);

    if (s) return s;
    if (e) return e;
    return 0;
}

int compare_intervals_s(void *a, void *b) {
    MyTimeInterval *i1, *i2;
    i1 = (MyTimeInterval*)a;
    i2 = (MyTimeInterval*)b;

    int s = MyTime_compare(&i1->start, &i2->start);
    if (s) return -s;
    return -1;
}

int compare_intervals_e(void *a, void *b) {
    MyTimeInterval *i1, *i2;
    i1 = (MyTimeInterval *)a;
    i2 = (MyTimeInterval *)b;

    int e = MyTime_compare(&i1->end, &i2->end);
    if (e) return e;
    return 1;
}

// int compare_interval_pairs(void *a, void *b) {
//     return compare_intervals(((IntervalPair)a)->interval, ((IntervalPair)b)->interval);
// }
// void destroy_interval_pairs(void *a) {
//     destroy_interval(((IntervalPair)a)->interval);
// }
// void *make_interval_pair(MyTimeInterval* interval) {
//     IntervalPair p = malloc(sizeof(*p));
//     p->interval = interval;
//     p->overlaps = 0;

//     return (void*)p;
// }

static bool interval_overlap(MyTimeInterval *i1, MyTimeInterval *i2) {
    MyTimeInterval *min = compare_intervals(i1, i2) <= 0 ? i1 : i2;
    MyTimeInterval *max = min == i1 ? i2 : i1;

    // they overlap only if the min's ending timestamp
    // is greater or equal to the max's starting timestamp
    return MyTime_compare(&min->end, &max->start) > 0;  
}

MyTimeInterval *merge_intervals(MyTimeInterval *intervals, int intervals_size, int *new_size) {
    PQ intervals_pq = pq_create(compare_intervals, NULL);    
    Stack interval_stack = stack_create(compare_intervals, destroy_interval);    

    // push all the intervals in a PQ
    for (int i = 0; i < intervals_size; i++) 
        pq_push(intervals_pq, make_interval(intervals[i].start, intervals[i].end));


    // The following procedure will merge all the time intervals (and note when overlaps happen)

    // While the PQ is not empty
    while (!pq_empty(intervals_pq)) {
        // first we take the min
        MyTimeInterval *interval = pq_pop(intervals_pq);
        if (!stack_empty(interval_stack)) {
            MyTimeInterval* first_interval = (MyTimeInterval*)stackNode_get_item(stack_get_first(interval_stack));
            // debbuging line
            assert(first_interval);
            // check if the interval and the first stack's interval overlap
            if (interval_overlap(interval, first_interval)) {
                // check if the ending timestamp is greater 
                // than the relative one in the first_interval
                // if it is updates stacks occurence
                if (MyTime_compare(&interval->end, &first_interval->end) > 0 ) 
                    MyTime_copy(&first_interval->end, interval->end);

                // free the interval since it's no longer needed
                destroy_interval(interval);
            
            } else {
                // if they don't overlap, then add the just-removed-interval to the stack
                stack_push(interval_stack, interval);
            }
        } else {
            stack_push(interval_stack, interval);
        }
    }

    // at this point the stack has all of the merged overlapping intervals
    // so we must add them to an array and return it
    MyTimeInterval *concurrent = calloc(stack_len(interval_stack), sizeof(*concurrent));
    int i = 0;
    while (!stack_empty(interval_stack)) {
        // pop the first element
        MyTimeInterval* interval = (MyTimeInterval*)stack_pop(interval_stack);
        assert(interval); // debbuging

        // insert the interval to the list
        MyTime_copy(&concurrent[i].start, interval->start);
        MyTime_copy(&concurrent[i].end, interval->end);
        // and increase the counter
        i ++;
        // destroy the interval since its no longer needed
        destroy_interval(interval);
    }
    
    pq_destroy(intervals_pq);
    stack_destroy(&interval_stack);

    *new_size = i;
    return concurrent;
}

MyTimeInterval *find_concurrent_intervals(MyTimeInterval **intervals, int intervals_size, int *interval_counters, int *concurrent_size) {
    //the compare returns > 0 for smaller and < 0 for greater so that we convert the min heap to a max heap
    PQ intervals_pq_max_start = pq_create(compare_intervals_s, NULL);
    PQ intervals_pq_min_end = pq_create(compare_intervals_e, NULL);
    MyTimeInterval *concurrent = NULL;
    int i = 0;
    // Stack interval_stack = stack_create(compare_interval_pairs, destroy_interval_pairs);    

    // push all the intervals in a PQ
    int size = 0;
    
    assert(pq_empty(intervals_pq_max_start));
    assert(pq_empty(intervals_pq_min_end));  
    for (int i = 0; i < intervals_size; i++) { 
        // if (i == k) continue;
        for (int j = 0; j < interval_counters[i]; j++) {
            // if the interval has some actual range
            if (MyTime_compare(&intervals[i][j].start, &intervals[i][j].end) != 0) {
                pq_push(intervals_pq_max_start, make_interval(intervals[i][j].start, intervals[i][j].end));
                pq_push(intervals_pq_min_end, make_interval(intervals[i][j].start, intervals[i][j].end));
                size++;
            }
        }
    }

    MyTimeInterval max_start[size];
    MyTimeInterval min_end[size];
    for (int i = 0; i < size; i++) {
        MyTimeInterval *i1, *i2;
        i1 = (MyTimeInterval *)pq_pop(intervals_pq_max_start);
        i2 = (MyTimeInterval *)pq_pop(intervals_pq_min_end);
        MyTime_copy(&max_start[i].start, i1->start);
        MyTime_copy(&max_start[i].end, i1->end);
        MyTime_copy(&min_end[i].start, i2->start);
        MyTime_copy(&min_end[i].end, i2->end);

        destroy_interval(i1);
        destroy_interval(i2);
    }
    
    int count_duplicates = 0;
    for (int max_start_i = 0; max_start_i < size; max_start_i++) {
        count_duplicates = 0;
        for (int min_end_i = 0; min_end_i < size; min_end_i++) {
            // printf("checking intervals [%d:%d - %d:%d] [%d:%d - %d:%d]\n",
            //        max_start[max_start_i].start.sec,
            //        max_start[max_start_i].start.millisec,
            //        max_start[max_start_i].end.sec,
            //        max_start[max_start_i].end.millisec,
            //        min_end[min_end_i].start.sec,
            //        min_end[min_end_i].start.millisec,
            //        min_end[min_end_i].end.sec, min_end[min_end_i].end.millisec);
            if (interval_overlap(&max_start[max_start_i], &min_end[min_end_i])) {
                // printf("They overlap\n");
                // if the intervals over lap create a new interval
                bool same = (compare_intervals(&max_start[max_start_i], &min_end[min_end_i]) == 0);
                if (same && !count_duplicates)
                    count_duplicates ++;
                else {    
                    i++;
                    MyTimeInterval *old = concurrent;
                    concurrent = calloc(i, sizeof(*concurrent));
                    if (i-1)
                        memcpy(concurrent, old, (i-1)*sizeof(MyTimeInterval));

                    MyTime start, end;
                    if (MyTime_compare(&max_start[max_start_i].start, &min_end[min_end_i].start) < 0) 
                        start = min_end[min_end_i].start;
                    else 
                        start = max_start[max_start_i].start;

                    if (MyTime_compare(&max_start[max_start_i].end, &min_end[min_end_i].end) > 0) 
                        end = min_end[min_end_i].end;
                    else 
                        end = max_start[max_start_i].end;                    

                    MyTime_copy(&concurrent[i-1].start, start);
                    MyTime_copy(&concurrent[i-1].end, end);
                    
                    // printf("Adding it : [%d:%d - %d:%d]\n",
                    //        concurrent[i - 1].start.sec,
                    //        concurrent[i - 1].start.millisec,
                    //        concurrent[i - 1].end.sec,
                    //        concurrent[i - 1].end.millisec);
                    free(old);
                }
            }
        }
    }
    
    // Now we have to merge all the time intervals
    
    pq_destroy(intervals_pq_max_start);
    pq_destroy(intervals_pq_min_end);
    // stack_destroy(&interval_stack);

    *concurrent_size = i;
    MyTimeInterval *merged = merge_intervals(concurrent, i, concurrent_size);
    free(concurrent);
    return merged;
}


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

static int cmp_time(int a, int b) {
    return a - b;
}

int MyTime_compare(MyTime *t1, MyTime *t2) {
    int cmp_h = cmp_time(t1->hour, t2->hour);
    int cmp_m = cmp_time(t1->min, t2->min);
    int cmp_s = cmp_time(t1->sec, t2->sec);
    int cmp_ms = cmp_time(t1->millisec, t2->millisec);

    if (cmp_h)
        return cmp_h;
    // cmp_h = 0
    if (cmp_m)
        return cmp_m;
    // cmp_m = 0
    if (cmp_s)
        return cmp_s;
    // cmp_s = 0
    return cmp_ms;
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
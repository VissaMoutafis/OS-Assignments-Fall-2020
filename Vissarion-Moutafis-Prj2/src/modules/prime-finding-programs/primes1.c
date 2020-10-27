#include <stdio.h>
#include <stdlib.h>
#include <sys/times.h>
#include <unistd.h>

#define YES 1
#define NO  0
int  prime(int n) {
    int i;
    if (n==1)  
        return(NO);
    for (i=2 ; i<n ; i++)
        if ( n % i == 0)  
            return(NO);
    return(YES);
}

int main(int argc, char *argv[]) {
    double t1, t2;
    struct tms tb1, tb2;
    double ticspersec;

    int lb = 0, ub = 0, i = 0;
    if ((argc != 3)) {
        printf("usage: prime1  lb ub\n");
        exit(1);
    }

    lb = atoi(argv[1]);
    ub = atoi(argv[2]);
    
    if ((lb < 1) || (lb > ub)) {
        printf("usage: prime1  lb ub\n");
        exit(1);
    }
    
    ticspersec = (double)sysconf(_SC_CLK_TCK);
    t1 = (double)times(&tb1);
    
    for (i = lb; i <= ub; i++) {
        if (prime(i) == YES) {
            t2 = times(&tb2);
            printf("%d,%.1f ", i, (float)(t2-t1)/(float)ticspersec);
        }
    }
    t2 = times(&tb2);
    printf("\t%.1f", (float)(t2-t1)/(float)ticspersec);
    printf("\n");
    fflush(stdout);

    return 0;
}
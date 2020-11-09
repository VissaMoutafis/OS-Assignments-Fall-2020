/*
** Implemented by Vissarion Moutafis
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/times.h>

#define YES 1
#define NO 0

int prime(int n) {
    if (n == 1)
        return NO;
    
    if (n%2 == 0 && n > 2)
        return NO;
    
    if (n%3 ==0 && n > 3)
        return NO;

    for (int i = 5; i*i <= n; i+=6) 
        if ((n%i == 0) || (n%(i+2) == 0))
            return NO;

    return YES;
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
    int c = 0;
    for (i = lb; i <= ub; i++) {
        if (prime(i) == YES) {
            c++;
            t2 = times(&tb2);
            printf("%d,%.1f ", i, (float)(t2-t1)/(float)ticspersec);
        }
    }
    t2 = times(&tb2);
    // if (c) {
        printf(":%.1f:", (float)(t2-t1)/(float)ticspersec);
        printf("\n");
    // }
    fflush(stdout);

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define YES 1
#define NO  0
int  prime(int n) {
    int i=0, limitup =0;
    limitup = (int)(sqrt((float)n));
    if (n==1)  
        return(NO);

    for (i=2 ; i  <= limitup ; i++)
        if ( n % i == 0)  
            return(NO);
    
    return(YES);
}
int  main(int argc , char *argv []) {
    int lb=0, ub=0, i=0;
    if ( (argc != 3) ) {
        printf("usage: prime1  lb ub\n");
        exit (1); 
    }
    
    lb=atoi(argv [1]);
    ub=atoi(argv [2]);
    
    if ( ( lb <1 )   || ( lb > ub ) ) {
        printf("usage: prime1  lb ub\n");
        exit (1); 
    }
    
    for (i=lb ; i  <= ub ; i++)
        if ( prime(i)==YES )
            printf("%d ",i);
    
    printf("\n");}
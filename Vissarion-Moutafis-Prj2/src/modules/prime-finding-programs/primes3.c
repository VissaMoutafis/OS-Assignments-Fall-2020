#include <stdio.h>
#include <stdlib.h>

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
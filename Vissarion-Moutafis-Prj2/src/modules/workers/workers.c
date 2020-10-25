#include "Process.h"
#include <time.h>
#include <string.h>

static char *posible_algos[] = {
    "./primes1",
    "./primes2",
    "./primes3",
};

char* choose_prime_algo(void) {
   
    int c=1;
    for (int i = 0; i < PRIME_ALGOS; i++) {
        c *= rand()%10;
    }

    return posible_algos[c%PRIME_ALGOS];
}

int main(int argc, char* argv[]) {
    srand(time(NULL));
    if (argc != 5) {
        fprintf(stderr, "Wrong input! ./external -l min -u max\n");
        exit(1);
    }

    char *bin_path = choose_prime_algo();
    char *args[] = {bin_path, argv[2], argv[4], (char*)0};

    if (execvp(bin_path, args) == -1) {
        perror("execvp()");
    }
    exit(1);
}
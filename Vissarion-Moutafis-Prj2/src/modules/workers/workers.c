#include "Process.h"
#include "ParsingUtils.h"
#include <time.h>

static char *posible_algos[] = {
    "./primes1",
    "./primes2",
    "./primes3",
};
int root_pid=-1;

void worker_handler(int sig, siginfo_t *siginfo, void *context) {
    // exit process
    exit(0);
    
}

// void read_from_child(int fd_array[][2], int number_of_children) {
//     struct pollfd fds[number_of_children];
//     int open_files = number_of_children;
//     //set up the fds array
//     for (int i = 0; i < number_of_children; i++) {
//         fds[i].events = POLLIN;
//         fds[i].fd = fd_array[i][READ];
//         fds[i].revents = 0;
//     }

//     while(open_files) {
//         // call the poll syscall
//         int retpoll = poll(fds, number_of_children, TIMEOUT);

//         if (retpoll > 0) {

//             for (int i = 0; i < number_of_children; i++)
//                 if (fds[i].revents & POLLIN) // if the file descriptor is in the ready list
//                     print_primes_from_child(fds[i].fd); // print what's in there
//                 else if (fds[i].revents & POLLHUP){
//                     fds[i].fd = -1;
//                     open_files--;
//                 }
//         } else if (retpoll < 0) {
//             perror("poll");
//             exit(1);
//         }
//     }
// }


static void check_args(int argc, char* argv[]) {
    if (argc != 9) {
        fprintf(stderr, "Wrong input! ./workers -l min -u max -algo algo num -ripd root pid\n");
        exit(1);
    }
    if( !( is_numeric(argv[2]) && is_numeric(argv[4]) && is_numeric(argv[6]) && is_numeric(argv[8])) ) {
        fprintf(stderr, "All args must be numeric.\n");
        exit(1);
    }
    if(!(atoi(argv[6]) <= PRIME_ALGOS)) {
        fprintf(stderr, "The algorithm inedx must be between 0 and %d.\n", PRIME_ALGOS-1);
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    check_args(argc, argv);
    root_pid = atoi(argv[8]);

    // int fd[2];
    // if (pipe(fd) == -1) {
    //     perror("pipe");
    //     exit(1);
    // }
    
    // if (fcntl(fd[WRITE], F_SETFL, fcntl(fd[WRITE], F_GETFL) | O_NONBLOCK) < 0) {
    //     perror("fcntl");
    //     exit(1);
    // }
    // if(fcntl(fd[READ], F_SETFL, fcntl(fd[READ], F_GETFL) | O_NONBLOCK) < 0) {
    //     perror("fcntl");
    //     exit(1);
    // }
    // // close the pipe write-end for parent
    // close(fd[WRITE]);

    // for the process that will execute
    pid_t child_pid = fork();
    if (child_pid == -1) {
        perror("fork");
        exit(1);
    }
    
    if(child_pid == 0) {
        // child case
        
        // close the pipe read-end for child
        // close(fd[READ]);        
        // dup2(fd[WRITE], STDOUT_FILENO);
        // close(fd[WRITE]);

        // choose the algorithm
        char *bin_path = posible_algos[atoi(argv[6])];
        // char *args[] = {};
        //execute the program
        if (execl(bin_path, bin_path, argv[2], argv[4], (char *)0) == -1) {
            perror("execvp()");
            exit(1);
        }
    }
    

    // read_from_child(&fd, 1);
    // wait for the child to end
    if ((child_pid = wait(NULL)) == -1) {
        char msg[20];
        sprintf(msg, "wait prime%s", argv[6]);
        perror(msg);
        exit(1);
    }
    printf("$");
    // ask parent to end stuff
    wait_signal_from(root_pid, SIGUSR1, worker_handler);
    
    exit(1);
}
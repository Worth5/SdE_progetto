#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    pid_t pid;
    if ((pid = fork()) < 0) {
        fprintf(stderr, "Error while forking. Exit\n");
        exit(EXIT_FAILURE);
    } 
    else if (pid == 0) {
        printf("This is the child process!\n");
    } 
    else {
        printf("This is the parent process!\n");
    }
    return 0;
}
 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    int var = 100;
    pid_t pid;
    if ((pid = fork()) < 0) {
        fprintf(stderr, "Error while forking. Exit\n");
        exit(EXIT_FAILURE);
    } 
    else if (pid == 0)
        var += 20;
    else
        var += 10;
    printf("PID = %d, var = %d\n", (int)getpid(), var);
    return 0;
}


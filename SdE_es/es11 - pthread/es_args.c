#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct st
{
    int index;
    int val;
};

// corpo del thread
void* print_args(void* args)
{
    struct st pars = *((struct st*)args);
    printf("Thread %d: val=%d \n", pars.index, pars.val);
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) 
{
    const int NUM_T = 5;
    pthread_t threads[NUM_T];
    struct st pars[NUM_T];
    
    // creazione dei thread
    int t, ret;
    for (t = 0; t < NUM_T; t++) 
    {
        printf("Creating thread %d \n", t);
        pars[t].index = t;
        pars[t].val = t + 100;
        ret = pthread_create(&threads[t], NULL, print_args, (void*)&pars[t]);
        if (ret != 0)
            fprintf(stderr, "error %d: cannot create thread", ret);
    }
    
    sleep(1);
    
    return EXIT_SUCCESS;
}

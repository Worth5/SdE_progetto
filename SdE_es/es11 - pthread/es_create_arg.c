#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// corpo del thread
void* print_hello(void* arg)
{
    int index = *((int*)arg);
    printf("Hello %d \n", index);
    pthread_exit(NULL);   // oppure: return NULL
}

int main(int argc, char* argv[]) 
{
    const int NUM_T = 5;
    pthread_t threads[NUM_T];
    
    // creazione di NUM_T thread
    int t, ret;
    for (t = 0; t < NUM_T; t++) 
    {
        printf("Creating thread %d \n", t);
        ret = pthread_create(&threads[t], NULL, print_hello, (void*)&t);
        if (ret != 0)
            fprintf(stderr, "error %d: cannot create thread", ret);
    }
    
    sleep(1);
    
    return EXIT_SUCCESS;
}

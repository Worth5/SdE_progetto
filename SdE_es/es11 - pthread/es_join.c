#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// corpo del thread
void* mult(void* arg)
{
    int index = *((int*)arg);
    long ret = index * 10;
    printf("Returning %d \n", ret);
    pthread_exit((void*)ret);
}

int main(int argc, char* argv[]) 
{
    int num_t;
    pthread_t* threads;
    int* index;
    void* status;  
    
    // il numero di thread e' letto da linea di comando
     if (argc == 2)
        num_t = atoi(argv[1]);
    else 
        num_t = 5;
        
    // allocazione dinamica delle strutture necessarie
    threads = (pthread_t*)malloc(num_t * sizeof(pthread_t));
    index = (int*)malloc(num_t * sizeof(int));
    
    // creazione di num_t thread
    int t, ret;
    for (t = 0; t<num_t; t++) 
    {
        printf("Creating thread %d \n", t);
        index[t] = t;
        ret = pthread_create(&threads[t], NULL, mult, (void*)&index[t]);
        if (ret != 0)
            fprintf(stderr, "error %d: cannot create thread", ret);
    }
    
    // attesa della terminazione di tutti i thread
    for (t = 0; t < num_t; t++) {
        ret = pthread_join(threads[t], &status);
        if (ret != 0) {
            fprintf(stderr, "error %d: cannot join", ret);
            exit(EXIT_FAILURE);
        }
        printf("Joined with thread %d, status=%d\n", t, (long)status);
    }
    
    free(threads);
    free(index);
    
    return EXIT_SUCCESS;
}

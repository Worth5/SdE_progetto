#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>

int var;
pthread_mutex_t mymutex = PTHREAD_MUTEX_INITIALIZER;

void* decr_var() 
{ 
    pthread_mutex_lock(&mymutex);
    if (var > 0)
    {
        sched_yield();
        var--;  
    }
    pthread_mutex_unlock(&mymutex);
    pthread_exit(NULL);
}


int main()
{
    pthread_t t1;
    pthread_t t2;
    
    var = 1;
    
    pthread_create(&t1, NULL, decr_var, NULL);
    pthread_create(&t2, NULL, decr_var, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("var = %d\n", var);
   
    return 0;
}
 

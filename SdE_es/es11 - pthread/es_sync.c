#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>

int var;

void* decr_var() 
{ 
    if (var > 0)
    {
        sched_yield();
        var--;  
    }
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
   
    return EXIT_SUCCESS;
}
 

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// variabile globale condivisa tra tutti i thread
unsigned int counter = 0; 

// thread che rimane in attesa di input da stdin e quando
// ne riceve uno resetta il contatore
void* listener()
{ 
    char c;
    while(scanf("%c",&c) == 1)
    {
        counter = 0;
        if (c == 'q')
            pthread_exit(NULL);  
    }
}

// thread che ogni secondo scrive su stdout il valore di un
// contatore e lo incrementa
void* writer()
{
    // scrive il valore corrente del contatore e lo incrementa 
    while(1)
    {
        printf("%d\n", counter);
        counter++;
        sleep(1);
    }
}

int main(int argc, char* argv[])
{
    pthread_t lt;
    pthread_t wt;

    // creazione di due thread
    pthread_create(&lt, NULL, listener, NULL);
    pthread_create(&wt, NULL, writer, NULL);

    // attesa per la terminazione del thread di ascolto
    pthread_join(lt, NULL);
    
    // la terminazione del main provoca il ritorno a __libc_start_main
    // che invoca exit(), percio' causa la terminazione di tutti i 
    // thread del processo
    printf("Termino il processo\n");
    return EXIT_SUCCESS;
}

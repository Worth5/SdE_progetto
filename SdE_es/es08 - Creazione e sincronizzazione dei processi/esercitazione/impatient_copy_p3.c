#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char* argv[])
{   
    if (argc != 3)
    {
        fprintf(stderr, "impatientcopy: numero argomenti errato\n");
        exit(EXIT_FAILURE); 
    }
    
    // creazione del processo figlio
    pid_t pid;
    if ((pid = fork()) < 0) 
    {
        fprintf(stderr, "impatientcopy: errore nella fork\n");
        exit(EXIT_FAILURE);
    } 
    else if (pid == 0)
    {
        // esecuzione del programma di copia
        execl("./slow_copy", "slow_copy", argv[1], argv[2], (char*)0); 
        
        // se arriviamo qui, c'e' stato un errore
        fprintf(stderr, "impatient_copy: errore nella exec\n");
        exit(EXIT_FAILURE);
    } 
    else 
    {
        // attesa terminazione del processo figlio
        if (wait(NULL) < 0) {
            fprintf(stderr, "impatient_copy: errore nella wait\n");
            exit(EXIT_FAILURE);
        }
       
        // ok, processo figlio e' terminato
        printf("Il processo figlio e' terminato\n");
    }
    
    return EXIT_SUCCESS;
}

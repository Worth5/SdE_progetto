/*
 * es_exec.c
 *
 * Questo programma crea un processo figlio, il quale 
 * esegue il programma esterno "somma"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main() 
{
    pid_t pid;
    int status;
 
    // numeri da sommare
    int a = 5;
    int b = 3;
    
    // per poter passare gli argomenti a exec e' necessario
    // effettuare una conversione a stringa
    char arg1[4];
    char arg2[4];
    sprintf(arg1, "%d", a);
    sprintf(arg2, "%d", b);
    
    // creazione processo figlio
    if ((pid = fork()) < 0) 
    {
        fprintf(stderr, "Errore durante la creazione del processo figlio\n");
        exit(EXIT_FAILURE);
    } 
    else if (pid == 0) 
    {
        execl("./somma", "somma", arg1, arg2, (char*)0);
                
        // si arriva qui solo se la exec fallisce
        printf("Errore nell'invocazione di exec()\n");
        exit(EXIT_FAILURE); 
    } 
    else 
    {
        // attende terminazione del processo figlio
        if (wait(NULL) < 0) 
        {
            fprintf(stderr, "Errore nella wait\n");
            exit(EXIT_FAILURE);
        }
        
        printf("Operazione completata\n");
    }
    return EXIT_SUCCESS;
}
 

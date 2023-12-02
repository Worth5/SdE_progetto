/*
 * es_pipe.c
 * 
 * Questo programma crea un processo con una fork, stabilisce una pipe tra processo
 * genitore e processo figlio. Il processo genitore utilizza la pipe per inviare
 * un messaggio al processo figlio.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>

int main() 
{    
    pid_t pid;
    const int BUFFLEN = 1024;
    char buff[BUFFLEN];
        
    // dichiarazione file descriptor della pipe
    int fd[2];
    
    // apertura della pipe
    if ( pipe(fd) < 0) 
    {  
        fprintf(stderr, "Errore nella creazione della pipe: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } 
    printf("Creata pipe: fd[0]=%d, fd[1]=%d \n", fd[0], fd[1]);
    
    // creazione del processo figlio
    pid = fork();
    if (pid < 0) 
    { 
        fprintf(stderr, "Errore nella creazione del processo figlio: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }  
    else if (pid > 0) {   // parent process 
        
        // chiusura read-end della pipe
        close(fd[0]);    
      
        // lettura messaggio da stdin
        printf("[Parent, pid=%d] Inserisci un messaggio per il figlio: ",getpid());
        if ( fgets(buff, BUFFLEN, stdin) == NULL)
        {
            fprintf(stderr, "Errore nella lettura da stdin\n");
            exit(EXIT_FAILURE);
        }
        
        // rimozione carattere di fine linea
        if (buff[strlen(buff)-1] == '\n')
            buff[strlen(buff)-1] = '\0';
        
        // scrittura sulla pipe
        write(fd[1], buff, strlen(buff)); 
        printf("[Parent, pid=%d] Messaggio inviato!\n",getpid());
        
        // chiusura write-end della pipe
        close(fd[1]);
        
        if (wait(NULL) < 0)
        {
            fprintf(stderr, "Errore nella wait\n");
            exit(EXIT_FAILURE);
        }
    }
    else {                // child process
        
        // chiusura write-end della pipe
        close(fd[1]);
        
        // lettura dalla pipe
        int n = read(fd[0], buff, BUFFLEN);
        buff[n] = '\0';   // aggiunta del carattere '\0' alla fine della stringa

        // stampa a video della stringa letta dalla pipe
        printf("[Child, pid=%d] Ricevuto messaggio: %s\n",getpid(), buff);
        
        // chiusura read-end della pipe
        close(fd[0]);
    } 
    return EXIT_SUCCESS; 
} 



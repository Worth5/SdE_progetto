/*
 * es_pipe_signal.c
 * 
 * Questo programma crea un processo con una fork, stabilisce una pipe tra processo
 * genitore e processo figlio. Il processo genitore utilizza la pipe per inviare
 * due messaggi al processo figlio. 
 * Il processo figlio, chiude la pipe subito dopo la ricezione del primo messaggio,
 * causando la generazione del segnale SIGPIPE sul processo genitore quando prova
 * a inviare il secondo messaggio sulla pipe.
 * Il segnale viene gestito con un handler.
 *
 * Cosa succede se si commenta la linea 43 (ovvero, se non si chiude il lato di 
 * lettura della pipe per il genitore)?
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

// handler per il segnale SIGPIPE
void sigpipe_handler(int signum) 
{
    fprintf(stderr, "Errore nella scrittura sulla pipe (%s)\n", strsignal(signum));
    exit(EXIT_FAILURE);
}

void parent_fun(int fd[])
{
    const int BUFFLEN = 1024;
    char buff[BUFFLEN];

    // registrazione handler per SIGPIPE
    if (signal(SIGPIPE, sigpipe_handler) == SIG_ERR) {
        fprintf(stderr, "Errore nella registrazione dell'handler: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }     

    // chiusura read-end della pipe
    close(fd[0]);    
    
    // lettura primo messaggio
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
            

    // lettura secondo messaggio
    usleep(1000);
    printf("[Parent, pid=%d] Inserisci un altro messaggio per il figlio: ",getpid());
    if ( fgets(buff, BUFFLEN, stdin) == NULL)
    {
        fprintf(stderr, "Errore nella lettura da stdin\n");
        exit(EXIT_FAILURE);
    }
    // rimozione carattere di fine linea
    if (buff[strlen(buff)-1] == '\n')
        buff[strlen(buff)-1] = '\0';
    
    // seconda scrittura sulla pipe
    write(fd[1], buff, strlen(buff)); 
    printf("[Parent, pid=%d] Messaggio inviato!\n",getpid());
    
    // chiusura write-end della pipe
    close(fd[1]);
}

void child_fun(int fd[])
{
    const int BUFFLEN = 1024;
    char buff[BUFFLEN];
    
    // chiusura write-end della pipe
    close(fd[1]);

    // lettura dalla pipe
    int n = read(fd[0], buff, BUFFLEN);
    buff[n] = '\0';
    
    // stampa a video della stringa letta dalla pipe
    printf("[Child, pid=%d] Ricevuto messaggio: %s\n",getpid(), buff);
    
    // chiusura read-end della pipe
    printf("[Child, pid=%d] Lato lettura della pipe chiuso\n",getpid());
    close(fd[0]);  
}

int main() 
{
    pid_t pid;
        
    // dichiarazione file descriptors della pipe
    int fd[2];
        
    // apertura della pipe
    if ( pipe(fd) < 0) {  
        fprintf(stderr, "Errore nella creazione della pipe: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } 
    
    // creazione del processo figlio
    pid = fork();
    if (pid < 0) 
    { 
        fprintf(stderr, "Errore nella creazione del processo figlio: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }  
    else if (pid > 0)   
    {
        // parent process 
        parent_fun(fd);
        
        // attendi terminazione del figlio
        if (wait(NULL) < 0)
        {
            fprintf(stderr, "Errore nella wait\n");
            exit(EXIT_FAILURE);
        }
    }
    else      
    {    
        // child process
        child_fun(fd);
    } 
    return 0; 
} 



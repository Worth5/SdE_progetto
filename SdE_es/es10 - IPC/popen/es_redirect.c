/*
 * es_redirect_ext.c
 * 
 * Questo programma crea un processo figlio con una fork e, usando una pipe, 
 * redirige lo standard output del processo genitore verso lo standard input
 * del processo figlio, che viene eseguito come programma esterno. 
 * Dopodiche' utilizza le funzioni per standard I/O per
 * inviare un messaggio da genitore a figlio
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int main() 
{    
    pid_t pid;
    int n;
    const int BUFFLEN = 1024;
    char buff[BUFFLEN];
    
    // dichiarazione file descriptors della pipe
    int fd[2];
    
    // apertura della pipe
    if (pipe(fd) < 0) {  
        fprintf(stderr, "Errore nella creazione della pipe: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } 
    
    // creazione del processo figlio
    if ( (pid = fork()) < 0) 
    { 
        fprintf(stderr, "Errore nella creazione del processo figlio: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }  
    else if (pid > 0) {   // parent process 
        
        // chiusura read-end della pipe
        close(fd[0]);    
        
        // scrittura sulla pipe
        printf("[Parent, pid=%d] Inserisci un messaggio per il proc. figlio...\n",getpid());
        fgets(buff, BUFFLEN, stdin);
        
        // mappa pipe su stdout
        dup2(fd[1], STDOUT_FILENO);
        
        // scrittura su stdout
        fputs(buff, stdout); 
        
        // chiusura write-end della pipe
        close(fd[1]);
    }
    else {                // child process
        
        // chiusura write-end della pipe
        close(fd[1]);
        
        // mappa pipe su stdin
        dup2(fd[0], STDIN_FILENO);

        execl("./readmsg", "readmsg", (char*)0);
    } 
    return 0; 
} 



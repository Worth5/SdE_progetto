/*
 * es_fifo_server_pp.c
 *
 * Prima di eseguire il programma, creare una FIFO dalla shell con il comando, e.g:
 * 
 *     mkfifo myfifo
 * 
 * Il programma (lato server) attende una richiesta dal client e invia una risposta.
 * 
 *     ./server myfifo 
 * 
 */ 


#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    
    const int NAMELEN = 256;
    char fifoname_request[NAMELEN];
    char fifoname_reply[NAMELEN];
    int fd_request;
    int fd_reply;
    
    const int BUFFLEN = 1024;
    char buff[BUFFLEN];
    int n;
    
    pid_t pid;
    
    if (argc != 2) {
        fprintf(stderr,"Errore, il programma deve avere un argomento, passati: %d\n", argc);
        exit(EXIT_FAILURE);
    }
    strcpy(fifoname_request, argv[1]);

    // apertura della FIFO
    if ( (fd_request = open(fifoname_request, O_RDWR)) < 0) {
        fprintf(stderr,"Errore nell'apertura della FIFO %s: %s", fifoname_request,strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    while(1)
    {
        printf("PID %d: in attesa di un messaggio...\n", getpid());
    
        // leggi la richiesta dalla FIFO
        if ( (n = read(fd_request, buff, BUFFLEN)) < 0) {
            fprintf(stderr,"Errore nella lettura della FIFO %s: %s", fifoname_request,strerror(errno));
            exit(EXIT_FAILURE);
        }
        //buff[n] = '\0';
    
        // ottieni il pid (prima stringa nel messaggio)
        sscanf(buff, "%d", &pid);
        printf("PID %d: ricevuta richiesta dal processo %d\n", getpid(), pid);
      
        // ottieni nome della fifo per la risposta come "fifo_reply.<CLIENT_PID>"
        snprintf(fifoname_reply, NAMELEN, "fifo_reply.%d", pid);
       
        // apertura della fifo per la risposta al client 
        if ( (fd_reply = open(fifoname_reply, O_RDWR)) < 0) {
            fprintf(stderr,"Errore nell'apertura della FIFO %s: %s", fifoname_reply, strerror(errno));
            exit(EXIT_FAILURE);
        }

        // invio della risposta al client dopo 5 secondi
        sleep(10);
        printf("PID %d: invio risposta al processo %d\n", getpid(), pid);
        if ( (write(fd_reply, "OK", 2)) != 2) {
            fprintf(stderr,"Errore nella scrittura sulla FIFO %s: %s", fifoname_reply,strerror(errno));
            exit(EXIT_FAILURE);
        }
        close(fd_reply);
    }
      

    close(fd_request);
    
    return 0;
}

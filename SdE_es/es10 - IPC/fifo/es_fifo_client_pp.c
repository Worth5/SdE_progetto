/*
 * es_fifo_client_pp.c
 *
 * Prima di eseguire il programma, creare una FIFO dalla shell con il comando, e.g:
 * 
 *     mkfifo myfifo
 * 
 * Il programma (lato client) invia una richiesta al server indicando il proprio PID
 * e attende la risposta.
 * 
 *     ./client myfifo 
 * 
 */ 

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char* argv[]) 
{    
    const int NAMELEN = 256;
    char fifoname_reply[NAMELEN];
    char fifoname_request[NAMELEN];
    int fd_reply;
    int fd_request;
    
    int n;
    const int MSGLEN = 1024;
    char msg[MSGLEN];
    
    if (argc != 2) {
        fprintf(stderr,"Errore, il programma deve avere un argomento, passati: %d\n", argc);
        exit(EXIT_FAILURE);
    }
    strcpy(fifoname_request,argv[1]);

    // ottieni nome della fifo per la risposta come "fifo_reply.<CLIENT_PID>"
    snprintf(fifoname_reply, NAMELEN, "fifo_reply.%d", getpid());
        
    // crea la nuova FIFO per ricevere la risposta
    int ret = mkfifo(fifoname_reply, S_IRUSR | S_IWUSR);
    if (ret < 0) 
    {
        fprintf(stderr,"Errore nella creazione della FIFO %s: %s", fifoname_reply,strerror(errno));   
        exit(EXIT_FAILURE);
    }
    printf("PID %d: creata FIFO %s\n", getpid(), fifoname_reply);
     
    // apertura della FIFO di risposta
    if ( (fd_reply = open(fifoname_reply, O_RDWR)) < 0) 
    {
        fprintf(stderr,"Errore nell'apertura della fifo %s: %s", fifoname_request,strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    // apertura della FIFO di richiesta
    if ( (fd_request = open(fifoname_request, O_RDWR)) < 0) {
        fprintf(stderr,"Errore nell'apertura della fifo %s: %s", fifoname_request,strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("PID %d: invio di una richiesta al server\n", getpid());
    
    // scrittura del messaggio sulla FIFO
    snprintf(msg, MSGLEN, "%d - Ciao, sono il processo %d", getpid(), getpid()); 
    if (write(fd_request, msg, strlen(msg)) != strlen(msg)) {
        fprintf(stderr,"Errore nella scrittura su FIFO %s: %s", fifoname_request,strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    // lettura della risposta dal server
    printf("PID %d: in attesa della risposta dal server...\n", getpid());
    if ( (n = read(fd_reply, msg, MSGLEN)) < 0) {
        fprintf(stderr,"Errore nella lettura dalla FIFO %s: %s", fifoname_reply,strerror(errno));
        exit(EXIT_FAILURE);
    }
    msg[n] = '\0';
    printf("PID %d: ricevuta risposta: %s\n", getpid(), msg);
    
    close(fd_request);
    close(fd_reply);
    
    // eliminazione della FIFO creata
    // unlink(fifoname_reply);
    
    return 0;
}

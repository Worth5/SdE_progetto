/*
 * es_fifo_client.c
 *
 * Prima di eseguire il programma, creare una FIFO dalla shell con il comando, e.g:
 * 
 *     mkfifo myfifo
 * 
 * Il programma (lato client) apre la fifo specificata come argomento e invia un  
 * un messaggio al client. 
 * Esempio di utilizzo:
 * 
 *     ./client myfifo 
 * 
 */ 


#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    
    char fifoname[256];
    int fd;
    
    const int MSGLEN = 512;
    char msg[MSGLEN];
    
    if (argc != 2) {
        fprintf(stderr,"Errore, il programma deve avere un argomento, passati: %d\n", argc);
        exit(EXIT_FAILURE);
    }
    strcpy(fifoname, argv[1]);

    // apertura della FIFO
    if ( (fd = open(fifoname, O_RDWR)) < 0) {
        fprintf(stderr,"Errore nell'apertura della fifo %s: %s", fifoname,strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    printf("PID %d: invio di un messaggio al server\n", getpid());
    
    // scrittura del messaggio sulla FIFO
    snprintf(msg, MSGLEN, "Ciao! Sono il client %d", getpid()); 
    if (write(fd, msg, strlen(msg)) != strlen(msg)) {
        fprintf(stderr,"Errore nella scrittura su FIFO %s: %s", fifoname,strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    close(fd);
    
    return 0;
}

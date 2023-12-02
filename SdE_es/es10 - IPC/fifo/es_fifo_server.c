/*
 * es_fifo_server.c
 *
 * Prima di eseguire il programma, creare una FIFO dalla shell con il comando, e.g:
 * 
 *     mkfifo myfifo
 * 
 * Il programma (lato server) apre la fifo specificata come argomento e attende di 
 * ricevere un messaggio dal client. Il messaggio ricevuto viene stampato a video.
 * Esempio di utilizzo:
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
    
    char fifoname[256];
    int fd;
    
    const int MSGLEN = 512;
    char msg[MSGLEN];
    int n;
    
    if (argc != 2) {
        fprintf(stderr,"Errore, il programma deve avere un argomento, passati: %d\n", argc);
        exit(EXIT_FAILURE);
    }
    strcpy(fifoname, argv[1]);

    // apertura della FIFO
    if ( (fd = open(fifoname, O_RDWR)) < 0) {
        fprintf(stderr,"Errore nell'apertura della FIFO %s: %s", fifoname,strerror(errno));
        exit(EXIT_FAILURE);
    }
      
    while(1)
    {  
        // attendi un messaggio
        printf("PID %d: in attesa di un messaggio...\n", getpid());
    
        if ( (n = read(fd, msg, MSGLEN)) < 0) {
            fprintf(stderr,"Errore nella lettura dalla FIFO %s: %s", fifoname,strerror(errno));
            exit(EXIT_FAILURE);
        }
        msg[n] = '\0';
    
        // stampa il messaggio ricevuto
        printf("Ricevuto: %s\n", msg);
    }      
    close(fd);
    
    return 0;
}

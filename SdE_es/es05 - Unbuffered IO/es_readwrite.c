/*
 * es_readwrite.c
 *
 * Questo programma mostra l'utilizzo delle system call open(), read(), 
 * write() e close().
 *
 * Il programma apre un file, ne legge i primi 32 byte e li scrive sullo 
 * standard output. Infine, aggiunge un messaggio alla fine del file.
 */

#include <fcntl.h>   // for open() 
#include <stdio.h>   // for printf() and stderr
#include <stdlib.h>  // for exit()
#include <errno.h>   // for errno
#include <string.h>  // for strerror
#include <unistd.h>  // for close()

// funzione che stampa su stderr una stringa di errore
void print_error(const char str[])
{
    fprintf(stderr, "errore durante operazione di %s [code %d, %s] \n", str, errno, strerror(errno));
}

int main()
{
    // dichiarazione del file descriptor 
    int fd;
    
    // variabili per read e write
    const int MAXBUFF = 32;     // dimensione del buffer
    char buff[MAXBUFF];         // buffer
    ssize_t rb, wb;             // numero caratteri letti/scritti
  
    
    // === OPEN === //
    // apertura del file di nome "fileditesto" in sola lettura
    fd = open("./fileditesto", O_RDONLY);
    if (fd == -1)
    {
        print_error("apertura");
        exit(EXIT_FAILURE);
    }
    printf ("fd %d aperto\n", fd);
   
    
    // ==== READ ==== //
    // lettura di MAXBUFF caratteri dal file
    if ( (rb = read(fd, buff, MAXBUFF)) < 0)
    {
        print_error("lettura");
        exit(EXIT_FAILURE);
    }
    printf ("Abbiamo letto %d byte dal file\n", (int)rb);
    

    // === CLOSE === //
    if (close(fd) == -1)
    {
        print_error("chiusura");
        exit(EXIT_FAILURE);
    }
    printf ("fd %d chiuso\n", fd);
    
   
    // === WRITE === //
    // scrittura di rb bytes dal buffer su stdout
    // NOTA: il file descriptor STDOUT_FILENO non deve essere 
    //       aperto esplicitamente
    if ( (wb = write(STDOUT_FILENO, buff, rb)) != rb)
    {
        print_error("scrittura");
        exit(EXIT_FAILURE);
    }
    printf ("\nAbbiamo scritto %d byte sul file\n", (int)wb);
    
     
    // === RE-OPEN === //
    // ri-apertura del file di nome "fileditesto" in scrittura per appendere
    // del testo alla fine 
    // NOTA: si riutilizza la variabile fd per il file descriptor
    fd = open("./fileditesto", O_WRONLY | O_APPEND);
    if (fd == -1)
    {
        print_error("apertura");
        exit(EXIT_FAILURE);
    }
    printf ("\nfd %d aperto\n", fd);
    
    
    // === WRITE === //
    // scrittura del contenuto del buffer su file
    const char msg[] = "Sono stato qui!!!";
    if ( (wb = write(fd, msg, strlen(msg))) != strlen(msg))
    {
        print_error("scrittura");
        exit(EXIT_FAILURE);
    }
    printf ("Abbiamo scritto %d byte sul file\n", (int)wb);
    
        // === CLOSE === //
    if (close(fd) == -1)
    {
        print_error("chiusura");
        exit(EXIT_FAILURE);
    }
    printf ("fd %d chiuso\n", fd);
    
    return 0;
}

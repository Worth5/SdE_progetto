/*
 * es_wordcount_unbuff.c
 * 
 * Questo programma legge un file di testo, ne conta
 * il numero di parole e scrive il risultato su standard
 * output.
 * Questa versione del programma utilizza le system call 
 * per l'unbuffered I/O.
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int main() 
{
    const char* inputfile = "bigfile";
    int fd;
    int word_count = 0;
    const int MAXBUFF = 4096;
    char buff[MAXBUFF];
    int read_bytes;
    
    // apertura del file in sola lettura
    fd = open(inputfile, O_RDONLY);
    if (fd < 0)
    {
        fprintf(stderr, "Errore nell'apertura del file (%s)\n",strerror(errno));
        exit(EXIT_FAILURE);
    }

    // lettura dal file fino a che non si raggiunge la fine
    while( (read_bytes = read(fd, buff, MAXBUFF)) > 0) 
    {
        for (int i = 0; i < read_bytes; i++)
        {
            if (buff[i] == ' ' || buff[i] == '\n')
                word_count++;
        }
    }
    
    // stampa del risultato
    fprintf(stdout, "Word count: %d\n", word_count);
    
    // chiusura del file descriptor
    close(fd);
    
    return EXIT_SUCCESS;
}

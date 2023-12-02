/*
 * es_copy.c
 * 
 * Questo programma implementa una versione semplificata del comando 'cp'.
 *
 * Usando le primitive per l'unbuffered I/O, legge dallo standard input 
 * e stampa i dati letti sullo standard output
 *
 * Esempio di utilizzo: ./mycp < input > output
 *
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() 
{
    // dichiarazione del buffer di appoggio
    const int BUFFSIZE = 4096;
    char buff[BUFFSIZE];
    int n;
    
    // leggi BUFFSIZE byte per volta da std input, fino a che
    // la read() non restituisce 0
    while ((n = read(STDIN_FILENO, buff, BUFFSIZE)) > 0) {
        // scrivi i byte contenuti in buff sullo std output
        if (write(STDOUT_FILENO, buff, n) != n) {
            fprintf(stderr, "Error in read(): %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    // se la read ha restituito un valore negativo, si e' verificato un errore
    if (n < 0) {
        fprintf(stderr, "Error in read(): %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}

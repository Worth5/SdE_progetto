/*
 * es_reverse.c
 *
 * Questo programma legge da standard input e scrive "specchiato" su
 * standard output.
 *
 * Esempio di utilizzo:
 *
 *     echo "aznalubma" > test 
 *     ./es_reverse < test
 *
 * Perche' la soluzione proposta non funziona se eseguiamo
 * il seguente comando?
 *
 *     echo "aznalubma" | ./es_reverse 
 *
 */

#include <fcntl.h>   // for open() 
#include <stdio.h>   // for printf() and stderr
#include <stdlib.h>  // for exit()
#include <errno.h>   // for errno
#include <string.h>  // for strerror
#include <unistd.h>  // for close()

int main()
{   
    // Lettura di un carattere per volta, a partire dall'ultimo
    // carattere nel file e andando a ritroso.
    // Il ciclo termina quando lseek() restituisce un valore negativo
    
    char buff;
    off_t i = -1;   
    while (lseek(STDIN_FILENO, i, SEEK_END) >= 0)
    {
        if (read(STDIN_FILENO, &buff, 1) < 0)
        {
            fprintf(stderr, "errore durante la lettura [code %d, %s] \n", errno, strerror(errno));
            exit(EXIT_FAILURE);
        }
        
        if (write(STDOUT_FILENO, &buff, 1) != 1)
        {
            fprintf(stderr, "errore durante la scrittura [code %d, %s] \n", errno, strerror(errno));
            exit(EXIT_FAILURE);
        }
        
        // spostamenteo sul carattere precedente
        i--;   
    }
    
    printf("\n");
    return EXIT_SUCCESS;
} 

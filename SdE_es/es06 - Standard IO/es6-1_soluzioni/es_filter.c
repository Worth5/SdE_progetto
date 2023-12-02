/*
 * es_filter.c
 * 
 * Questo programma legge da standard input una sequenza di interi e li scrive su 
 * standard output. 
 * Eventuali caratteri diversi da numeri interi vengono scartati.
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

int main() 
{
    int n, ret;
    char c;

    // prova a leggere dal file fino a che non si raggiunge la fine
    while( (ret = fscanf(stdin, "%d", &n)) != EOF) 
    {
        if (ret > 0)
        {
            // e' stato letto un numero intero
            // scrivilo su stdout
            fprintf(stdout, "%d\n",n);  // equivale a: printf("%d\n",n);
        }
        else
        {
            // non e' stato letto un numero intero
            // prova a leggere un carattere per "consumare" il prossimo byte 
            // nello stream e scorrere il file
            if (fscanf(stdin, "%c", &c) == EOF)
            {
                // se non si e' in grado di leggere un carattere, termina con errore
                fprintf(stderr,"Errore: %s\n",strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
    }
    
    // controlla se ciclo terminato con errore prima della fine del file
    if (!feof(stdin))
    {
        fprintf(stderr,"Errore: %s\n",strerror(errno));
        exit(EXIT_FAILURE);
    }
      
    return EXIT_SUCCESS;
}

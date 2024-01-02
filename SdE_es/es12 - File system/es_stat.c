/*
 * es_stat.c
 *
 * Questo programma controlla se il file specificato come argomento
 * e' un file regolare. In caso affermativo, scrive su standard output
 * la sua dimensione.
 *
 * Verificare, tramite il comando "ls -l" oppure "stat" che la dimensione
 * visualizzata sia corretta
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Numero di argomenti errato\n");
        exit(EXIT_FAILURE);
    }
    
    char* file_name = (char*)malloc(strlen(argv[1]));
    strcpy(file_name, argv[1]);
   
    // recupero dei metadati del file
    struct stat file_stat;
    if ( stat(file_name, &file_stat) < 0)
    {
        fprintf(stderr, "Errore nella lettura delle informazioni del file %s\n", file_name);
        exit(EXIT_FAILURE);
    }
    
    // se il file e' un file regolare, visualizza la sua dimensione
    if (S_ISREG(file_stat.st_mode) > 0)
    {
        off_t file_size = file_stat.st_size;
        printf("Il file e' grande %d byte\n", file_size);
    }
    else
    {
        printf("Il file non e' un file regolare\n");
    }
    
    return EXIT_SUCCESS;
}

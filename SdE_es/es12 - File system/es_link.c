/*
 * es_link.c
 *
 * Questo programma crea un secondo hard link per il file specificato
 * come parametro, e attende che l'utente prema un tasto qualsiasi 
 * prima di eliminare il precedente hard link.
 * Quando l'utente preme un altro tasto, il programma rimuove anche
 * il secondo hard link, e quindi il file
 *
 * Tra un'operazione e l'altra, aprire un'altra finestra del terminale
 * e vedere cosa cambia nell'output di "ls -l" e "df ."
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Numero di argomenti errato\n");
        exit(EXIT_FAILURE);
    }
    
    size_t len1 = strlen(argv[1]);
    size_t len2 = strlen(argv[2]);
    
    // allocazione memoria dinamica e copia stringhe
    char* old_link = (char*)malloc(len1);
    char* new_link = (char*)malloc(len2);
    strcpy(old_link, argv[1]);
    strcpy(new_link, argv[2]);

    printf("Creazione di un hard link per il file di nome file1\n");
    if ( link(old_link, new_link) < 0)
    {
        fprintf(stderr, "Errore nella creazione dell'hard link\n");
        exit(EXIT_FAILURE);
    }
    
    // rimani in pausa fino a che l'utente non preme un tasto
    getchar();
    
    printf("Eliminazione del primo hard link\n");
    if ( unlink(old_link) < 0)
    {
        fprintf(stderr, "Errore nella rimozione dell'hard link\n");
        exit(EXIT_FAILURE);
    }
    
    // rimani in pausa fino a che l'utente non preme un tasto
    getchar();
    
    printf("Eliminazione del secondo hard link\n");
    if ( unlink(new_link) < 0)
    {
        fprintf(stderr, "Errore nella rimozione dell'hard link\n");
        exit(EXIT_FAILURE);
    }
    
    free(old_link);
    free(new_link);
    return EXIT_SUCCESS;
}


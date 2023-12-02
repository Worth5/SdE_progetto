/* 
 * es_getenv.c
 * 
 * Questo programma scrive la stringa passata dalla linea
 * di comando in un file chiamato "destination.txt" all'interno
 * della directory HOME dell'utente.
 */

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    FILE* fp;
    const int MAXLEN = 100;
    char str[MAXLEN];

    // controllo validita' argomenti
    if (argc == 1 || argc > 2)
    {
       fprintf(stderr, "Numero errato di argomenti\n");
       exit(EXIT_FAILURE);
    }

    // creazione del percorso di destinazione
    sprintf(str,"%s/destination.txt", getenv("HOME"));

    printf("Scrivo la stringa sul file %s...\n", str);
    if ((fp = fopen(str, "w")) == NULL)
    {
        fprintf(stderr,"Errore nell'apertura dello stream\n");
        exit(EXIT_FAILURE);
    }
    if (fputs(argv[1], fp) == EOF)
    {
        fprintf(stderr,"Errore nella scrittura sullo stream\n");
        exit(EXIT_FAILURE);
    }
    fclose(fp);

    return EXIT_SUCCESS;
}

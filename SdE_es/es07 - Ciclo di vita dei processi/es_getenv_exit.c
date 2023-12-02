/* 
 * es_getenv_exit.c
 * 
 * Questo programma scrive la stringa passata dalla linea
 * di comando in un file chiamato "destination.txt" all'interno
 * della directory HOME dell'utente.
 * Al termine, scrive un messaggio su standard output che informa
 * l'utente del corretto funzionamento
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void fun()
{
    printf("Scrittura del file terminata con successo\n");
}

int main(int argc, char* argv[])
{
    FILE* fp;
    const int MAXLEN = 100;
    char str[MAXLEN];

    // registrazione dell'handler di uscita
    if ( atexit(&fun) != 0)
    {
        fprintf(stderr, "Errore nella registrazione dell'handler di uscita\n");       
        _exit(EXIT_FAILURE);
    }
    
    // controllo validita' argomenti
    if (argc == 1 || argc > 2)
    {
       fprintf(stderr, "Numero errato di argomenti\n");
       _exit(EXIT_FAILURE);
    }

    // creazione del percorso di destinazione
    sprintf(str,"%s/destination.txt", getenv("HOME"));

    printf("Scrivo la stringa sul file %s...\n", str);
    if ((fp = fopen(str, "w")) == NULL)
    {
        fprintf(stderr,"Errore nell'apertura dello stream\n");
        _exit(EXIT_FAILURE);
    }
    if (fputs(argv[1], fp) == EOF)
    {
        fprintf(stderr,"Errore nella scrittura sullo stream\n");
        _exit(EXIT_FAILURE);
    }
    fclose(fp);

    return EXIT_SUCCESS;
}

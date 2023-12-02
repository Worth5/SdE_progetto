/*
 * es_wordcount_fscanf.c
 * 
 * Questo programma legge un file di testo, ne conta
 * il numero di parole e scrive il risultato su standard
 * output.
 * Questa versione del programma legge una stringa per 
 * volta usando la funzione fscanf()
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int main() 
{
    const char* inputfile = "bigfile";
    FILE* fp;
    int word_count = 0;
    const int MAXLEN = 4096;
    char line[MAXLEN];

    // apertura del file in sola lettura
    fp = fopen(inputfile, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Errore nell'apertura del file (%s)\n",strerror(errno));
        exit(EXIT_FAILURE);
    }

    // lettura dal file fino a che non si raggiunge la fine
    while(fscanf(fp, "%s", line) > 0) 
    {
        word_count++;
    }
    
    // stampa del risultato
    fprintf(stdout, "Word count: %d\n", word_count);
    
    // chiusura dello stream
    fclose(fp);
    
    return EXIT_SUCCESS;
}

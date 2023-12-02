/*
 * es_wordcount_fgetc.c
 * 
 * Questo programma legge un file di testo, ne conta
 * il numero di parole e scrive il risultato su standard
 * output.
 * Questa versione del programma legge un carattere per 
 * volta usando la funzione fgetc()
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
    int ret;
    char c;
    
    // apertura del file in sola lettura
    fp = fopen(inputfile, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Errore nell'apertura del file (%s)\n",strerror(errno));
        exit(EXIT_FAILURE);
    }

    // lettura dal file fino a che non si raggiunge la fine
    while( (ret = fgetc(fp)) != EOF) 
    {
        c = ret;
        if (c == ' ' || c == '\n')
            word_count++;
    }
    
    // stampa del risultato
    fprintf(stdout, "Word count: %d\n", word_count);
 
    // chiusura dello stream
    fclose(fp);
       
    return EXIT_SUCCESS;
}

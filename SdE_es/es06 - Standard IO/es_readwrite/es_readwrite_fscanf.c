/*
 * es_readwrite_fscanf.c
 * 
 * Questo programma apre un file in sola lettura e ne legge
 * il contenuto usando fscanf(). Legge numeri interi e li 
 * scrive su un secondo file aperto in scrittura. 
 * Quando incontra un carattere diverso da un numero intero,
 * lo stampa a video e termina
 */ 

#include <stdio.h>
#include <stdlib.h>

int main() 
{
    FILE* fp_in;
    FILE* fp_out;
    const char filename_in[] = "numbers.txt";
    const char filename_out[] = "numbers_filtered.txt";
    
    // stringa che contiene la linea letta da file di input
    const int MAX_LINE = 1024;
    char str[MAX_LINE];   
          
    // apre file sorgente in sola lettura
    if ((fp_in = fopen(filename_in, "r")) == NULL)
    {
        fprintf(stderr,"ERROR while opening %s\n", filename_in); 
        exit(EXIT_FAILURE);          
    }
    
    // apre file destinazione in sola scrittura
    if ((fp_out = fopen(filename_out, "w")) == NULL)
    {
        fprintf(stderr,"ERROR while opening %s\n", filename_out); 
        exit(EXIT_FAILURE);          
    }
    
    // legge il prossimo numero intero
    int n;
    while (fscanf(fp_in, "%d", &n) > 0) 
    {
        // scrive il numero intero letto su fp_out
        if (fprintf(fp_out, "%d\n", n) == 0) 
        {
            fprintf(stderr,"ERROR while writing to %s\n", filename_out); 
            exit(EXIT_FAILURE);
        }
    }
    
    // prova a leggere un carattere
    char c;
    if (fscanf(fp_in, "%c", &c) > 0)
        fprintf(stdout, "Hai incontrato il carattere %c\n",c);
    else
        fprintf(stdout, "Abbiamo raggiunto la fine del file\n");
    
    // chiude gli stream
    fclose(fp_in);
    fclose(fp_out);
    
    return EXIT_SUCCESS;
}

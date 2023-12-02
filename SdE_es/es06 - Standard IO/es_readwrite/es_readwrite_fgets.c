/*
 * es_readwrite_line.c
 * 
 * Questo programma apre un file in sola lettura e lo legge una linea alla volta.
 * Le linee lette vengono copiate in un secondo file, aperto in sola scrittura (senza append).
 * 
 * Verificare che il file ottenuto sia identico all'originale. Quali sono i permessi sul nuovo file?
 */ 

#include <stdio.h>
#include <stdlib.h>

int main() 
{
    FILE* fp_in;
    FILE* fp_out;
    const char filename_in[] = "pp.txt";
    const char filename_out[] = "pp_copy.txt";
    
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
    
    // legge da fp_in fino a che ci sono linee da leggere
    while (fgets(str, MAX_LINE, fp_in) != NULL) 
    {
        // scrive la linea letta su fp_out
        if (fputs(str, fp_out) == EOF) 
        {
            fprintf(stderr,"ERROR while writing to %s\n", filename_out); 
            exit(EXIT_FAILURE);
        }
    }
    
    fclose(fp_in);
    fclose(fp_out);
    
    return EXIT_SUCCESS;
}

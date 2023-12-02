/*
 * es_readwrite_char.c
 * 
 * Questo programma apre un file in sola lettura e lo legge un carattere alla volta.
 * I caratteri letti vengono copiati in un secondo file, aperto in sola scrittura (senza append).
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
    int n;
    char ch;
       
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
    
    // legge da fp_in fino a che ci sono caratteri da leggere
    while((n = fgetc(fp_in)) != EOF)
    { 
        // ora possiamo convertirlo a char perche' siamo sicuri
        // che fgetc e' terminata senza errori
        ch = n;   
        
        // scrive il carattere letto su fp_out
        // ch viene convertito a intero (ok, non si perde informazione)
        if (fputc(ch, fp_out) == EOF)
        {  
            fprintf(stderr,"ERROR while writing to %s\n", filename_out); 
            exit(EXIT_FAILURE);
        }
    }
    
    fclose(fp_in);
    fclose(fp_out);
    
    return EXIT_SUCCESS;
}

/*
 * es_echo_form.c
 * 
 * Questo programma apre un file in sola lettura, legge una riga 
 * usando fgets() e la scrive su standard output. 
 */ 

#include <stdio.h>
#include <stdlib.h>

int main() 
{
    const int MAXLEN = 100;
    char str[MAXLEN];
    
    FILE* fp_in;
    const char filename_in[] = "linea.txt";
    
    // apre file sorgente in sola lettura
    if ((fp_in = fopen(filename_in, "r")) == NULL)
    {
        fprintf(stderr,"ERROR while opening %s\n", filename_in); 
        exit(EXIT_FAILURE);          
    }
    
    // leggi da file
    fgets(str, MAXLEN, fp_in); 
    
    // scrivi su standard output
    fputs(str, stdout);
    
    return EXIT_SUCCESS;
}

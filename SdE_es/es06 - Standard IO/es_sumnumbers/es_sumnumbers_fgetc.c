/*
 * es_sumnumbers_fgetc.c
 * 
 * Questo programma apre un file in sola lettura e legge i 
 * numeri interi presenti nel file usando la funzione 
 * fgetc(). Infine, scrive la somma dei numeri letti sullo
 * standard output. 
 */ 

#include <stdio.h>
#include <stdlib.h>

int main() 
{
    FILE* fp_in;
    const char filename_in[] = "numbers.txt";
           
    // variabile che mantiene e aggiorna la somma dei numeri letti
    int somma = 0;
           
    // apre file sorgente in sola lettura
    if ((fp_in = fopen(filename_in, "r")) == NULL)
    {
        fprintf(stderr,"ERROR while opening %s\n", filename_in); 
        exit(EXIT_FAILURE);          
    }
    
    // array che contiene i caratteri letti prima di incontrare uno spazio
    char new_number[5];
    int i = 0;
    
    // legge il prossimo numero intero
    int n;
    while ( (n = fgetc(fp_in)) > 0) 
    {
        char ch = n;
        if (ch != ' ' && ch != '\n')
        {
            // aggiungiamo una cifra al numero
            new_number[i] = ch;
            i++;
        }
        else
        {
            // abbiamo letto completamente un numero
            new_number[i] = '\0';
            somma = somma + atoi(new_number);
            i = 0;
        }
    }
    
    printf("La somma e' %d\n", somma); 
           
    // chiude lo stream
    fclose(fp_in);
    
    return EXIT_SUCCESS;
}

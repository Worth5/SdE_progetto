/*
 * es_sumnumbers_form.c
 * 
 * Questo programma apre un file in sola lettura e legge i 
 * numeri interi presenti nel file usando la funzione 
 * fscanf(). Infine, scrive la somma dei numeri letti sullo
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
    
    // legge il prossimo numero intero
    int n;
    while (fscanf(fp_in, "%d", &n) > 0) 
        somma = somma + n;
        
    printf("La somma e' %d\n", somma); 
    
    // chiude lo stream
    fclose(fp_in);
    
    return EXIT_SUCCESS;
}

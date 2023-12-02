/*
 * es_formatted_input.c
 */ 

#include <stdio.h>
#include <stdlib.h>


void leggi_da_tastiera();
void leggi_da_file();
void leggi_da_stringa();


int main() 
{
    leggi_da_tastiera();
    printf("==================\n");
    leggi_da_file();
    printf("==================\n");
    leggi_da_stringa();

    return EXIT_SUCCESS;
}


void leggi_da_tastiera()
{
    int dd, mm, aaaa;
    scanf("%d/%d/%d", &dd, &mm, &aaaa);
    
    printf("Giorno: %02d \nMese: %02d \nAnno: %04d \n", dd, mm, aaaa);
}

void leggi_da_file()
{
    int dd, mm, aaaa;
    FILE* fp;
    if ( (fp = fopen("data_galileo.txt", "r")) == NULL)
    {
        fprintf(stderr, "Errore nell'apertura del file\n"); 
        exit(EXIT_FAILURE);
    }
    
    fscanf(fp, "%d/%d/%d", &dd, &mm, &aaaa);
    
    printf("Giorno: %02d \nMese: %02d \nAnno: %04d \n", dd, mm, aaaa);
    
    fclose(fp);
}

void leggi_da_stringa()
{
    int dd, mm, aaaa;
    const char str[] = "15/02/1564";
 
    sscanf(str, "%d/%d/%d", &dd, &mm, &aaaa);
       
    printf("Giorno: %02d \nMese: %02d \nAnno: %04d \n", dd, mm, aaaa);   
}


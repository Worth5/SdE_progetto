#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) 
{
    // verifica argomenti
    if (argc != 3)
    {
        fprintf(stderr, "slow_copy: numero argomenti errato\n");
        exit(EXIT_FAILURE);
    }   
    
    // apertura file di input e output
    FILE* fp_in = fopen(argv[1], "r");
    FILE* fp_out = fopen(argv[2], "w");   
       
    // legge da file di input un carattere alla volta
    int c;
    while ((c = fgetc(fp_in)) != EOF) 
    {
        // scrive il carattere letto su file di output
        if (fputc(c, fp_out) == EOF) 
        {
            fprintf(stderr,"slow_copy: errore durante la scrittura\n"); 
            exit(EXIT_FAILURE);
        }
    }
    
    if (ferror(fp_in) > 0)
    {
        fprintf(stderr,"slow_copy: errore durante la lettura\n"); 
        exit(EXIT_FAILURE);
    }
    
    fclose(fp_in);
    fclose(fp_out);
    
    return EXIT_SUCCESS;
}

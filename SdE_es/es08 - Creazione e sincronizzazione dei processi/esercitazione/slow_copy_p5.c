#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

// contatore dei caratteri copiati
unsigned int copiati;

// definizione dell'handler del segnale SIGALRM
void signal_hdlr(int signo)
{
    // controlla quale segnale ha causato l'esecuzione
    // dell'handler
    
    if (signo == SIGALRM)
    {
        // se SIGALRM, stampa del contatore e reset del timer
        printf("copiati %d caratteri\n", copiati); 
        alarm(1);
    }
    else 
    {
        // se SIGTERM, termina processo con fallimento
        printf("terminazione forzata dal genitore\n");
        exit(EXIT_FAILURE); 
    }
}

int main(int argc, char* argv[]) 
{
    // verifica argomenti
    if (argc != 3)
    {
        fprintf(stderr, "slow_copy: numero argomenti errato\n");
        exit(EXIT_FAILURE);
    }   
    
    // registrazione dell'handler per SIGALRM
    if (signal(SIGALRM, signal_hdlr) == SIG_ERR)
    {
        fprintf(stderr, "slow_copy: registrazione handler SIGALRM fallita\n");
        exit(EXIT_FAILURE);
    }
 
    // registrazione dell'handler per SIGTERM
    // NOTA: l'handler e' lo stesso di SIGALRM
    if (signal(SIGTERM, signal_hdlr) == SIG_ERR)
    {
        fprintf(stderr, "slow_copy: registrazione handler SIGTERM fallita\n");
        exit(EXIT_FAILURE);
    }   
 
    // apertura file di input e output
    FILE* fp_in = fopen(argv[1], "r");
    FILE* fp_out = fopen(argv[2], "w");   
       
    // inizializzazione contatore e avvio timer
    copiati = 0;   
    alarm(1);
       
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
        
        // incrementa contatore dei caratteri copiati
        copiati++;
    }
    if (ferror(fp_in) > 0)
    {
        fprintf(stderr,"slow_copy: errore durante la lettura\n"); 
        exit(EXIT_FAILURE);
    }
    
    printf("Copia di %d caratteri terminata\n", copiati);
    
    fclose(fp_in);
    fclose(fp_out);
    
    return EXIT_SUCCESS;
}

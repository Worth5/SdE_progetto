#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

void signal_handler(int signo)
{
    printf("Catturato segnale %d (%s)!\n", signo, strsignal(signo));
    
    // strsignal() descrive il tipo di segnale 
    // analoga a strerror() per variabile errno
}

void sigsegv_handler(int signo)
{
    printf("Accesso a puntatore errato (%s)!\n", strsignal(signo));
    exit(EXIT_FAILURE);
}

int main()
{
    // registrazione degli handler dei segnali

    if (signal(SIGINT, SIG_IGN) == SIG_ERR) 
    {
        fprintf(stderr, "Registrazione handler SIGINT non riuscita\n");
        exit(EXIT_FAILURE);
    }

    if (signal(SIGTERM, signal_handler) == SIG_ERR) 
    {
        fprintf(stderr, "Registrazione handler SIGTERM non riuscita\n");
        exit(EXIT_FAILURE);
    }
    
    if (signal(SIGSEGV, sigsegv_handler) == SIG_ERR) 
    {
        fprintf(stderr, "Registrazione handler SIGSEGV non riuscita\n");
        exit(EXIT_FAILURE);
    }
 
    // decommenta queste linee per provocare invio di SIGSEGV    
//    int* mem = NULL;
//    printf("%d\n", *mem);
    
    printf("Esecuzione di un ciclo infinito...\n");
  
    while(1);
    
    printf("Non puoi essere qui!\n");
    return EXIT_SUCCESS;
}

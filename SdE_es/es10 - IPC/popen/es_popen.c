/* 
 * es_popen.c 
 * 
 * In questo programma il processo genitore redirige il proprio standard
 * output verso lo standard input del processo figlio utilizzando la 
 * funzione popen()
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() 
{    
    FILE* fpipe;
    const int MAXLEN = 1024;
    char buff[MAXLEN];

    // apertura di una pipe in scrittura verso il processo indicato da linea di comando
    if ((fpipe = popen("./readmsg", "w")) == NULL) {
        fprintf(stderr,"Errore nell'apertura della pipe verso processo figlio: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // lettura di una linea da stdin e scrittura della linea
    // sulla pipe. Il processo ricevitore la riceve su stdin
    printf("[Parent, pid=%d] Inserisci un messaggio per il proc. figlio...\n",getpid());
    fgets(buff, MAXLEN, stdin);

    fputs(buff, fpipe);
    
    // chiusura della pipe
    if (pclose(fpipe) == -1) 
    {
        fprintf(stderr,"Errore durante pclose");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}

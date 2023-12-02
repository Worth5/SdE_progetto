#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Numero errato di argomenti\n");
        exit(EXIT_FAILURE);
    }
  
    // ottieni durata del timer in secondi
    int seconds = atoi(argv[1]);
    
    // invia segnale
    if (alarm(seconds) < 0)
    {
        fprintf(stderr, "alarm non riuscita (%s)\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    while(1);
    
    return EXIT_SUCCESS;
}

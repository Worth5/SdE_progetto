#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Numero errato di argomenti\n");
        exit(EXIT_FAILURE);
    }
  
    // ottieni pid da terminare da shell
    int pid = atoi(argv[1]);
    
    // invia segnale SIGKILL al processo
    if (kill(pid, SIGKILL) < 0)
    {
        fprintf(stderr, "kill non riuscita (%s)\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    return EXIT_SUCCESS;
}

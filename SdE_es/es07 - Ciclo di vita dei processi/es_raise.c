#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

void sig_hdlr(int signo)
{
    printf("Catturato segnale %d (%s)\n", signo, strsignal(signo));
    exit(EXIT_SUCCESS);
}

int main()
{
    if (signal(SIGTERM, sig_hdlr) == SIG_ERR)
    {
        fprintf(stderr, "signal non riuscita\n");
        exit(EXIT_FAILURE);
    }
    
    // invia segnale
    if (raise(SIGTERM) < 0)
    {
        fprintf(stderr, "raise non riuscita\n");
        exit(EXIT_FAILURE);
    }
    
    return EXIT_SUCCESS;
}

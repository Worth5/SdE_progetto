#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

// variabile globale per fare in modo che l'handler
// del segnale possa utilizzarla
pid_t pid;
    
// handler del segnale SIGALRM
void sigalarm_hdlr(int signo)
{
    // invia SIGTERM al processo figlio
    kill(pid, SIGTERM);
    printf("Limite di tempo raggiunto\n");
}

int main(int argc, char* argv[])
{   
    unsigned int timer;
    
    if (argc != 4)
    {
        fprintf(stderr, "impatientcopy: numero argomenti errato\n");
        exit(EXIT_FAILURE); 
    }
    timer = atoi(argv[3]);
    
    // registrazione dell'handler per segnale SIGALRM
    if (signal(SIGALRM, sigalarm_hdlr) == SIG_ERR) 
    {
        fprintf(stderr, "impatientcopy: registrazione handler SIGALRM non riuscita\n");
        exit(EXIT_FAILURE);
    }
    
    // creazione del processo figlio
    if ((pid = fork()) < 0) 
    {
        fprintf(stderr, "impatientcopy: errore nella fork\n");
        exit(EXIT_FAILURE);
    } 
    else if (pid == 0)
    {
        // esecuzione del programma di copia
        execl("./slow_copy", "slow_copy", argv[1], argv[2], (char*)0); 
        
        // se arriviamo qui, c'e' stato un errore
        fprintf(stderr, "impatient_copy: errore nella exec\n");
        exit(EXIT_FAILURE);
    } 
    else 
    {
        // imposta timer per tempo massimo
        alarm(timer);
        
        // attesa terminazione del processo figlio
        if (wait(NULL) < 0) {
            fprintf(stderr, "impatient_copy: errore nella wait\n");
            exit(EXIT_FAILURE);
        }
       
        // ok, processo figlio e' terminato
        printf("Il processo figlio e' terminato\n");
    }
    
    return EXIT_SUCCESS;
}

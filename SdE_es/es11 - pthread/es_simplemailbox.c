/*
 * es_simplemailbox.c
 *
 * Questo programma gestisce una semplice mailbox che puo'
 * contenere al piu' un messaggio.
 * Un thread prova a leggere il messaggio nella mailbox e, 
 * se la mailbox e' vuota, si mette in attesa che diventi piena.
 * Un altro thread attende l'input da parte dell'utente e lo 
 * inserisce nella mailbox, risvegliando l'eventuale thread in
 * attesa.
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int presente = 0;
char* msg;

pthread_mutex_t mymutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t mycond = PTHREAD_COND_INITIALIZER;

void* read_message() 
{ 
    // test della condizione
    pthread_mutex_lock(&mymutex);
    if (presente == 0)
    { 
        // se non verificata, attendo
        printf("read_message: non ci sono messaggi, attendo...\n");
        pthread_cond_wait(&mycond, &mymutex);
    }
    // aggiorno la risorsa e rilascio il mutex
    presente--;
    pthread_mutex_unlock(&mymutex);
    
    printf("read_message: letto %s\n",msg);
    pthread_exit(NULL);
}
 
void* write_message() 
{ 
    const int MAXLEN = 1024;
    char input[MAXLEN];
    
    printf("write_message: in attesa che l'utente lasci un messaggio...\n");
    fgets(input, MAXLEN, stdin);
    
    // alloco memoria adeguata per la stringa condivisa 
    // e copio la stringa letta da tastiera
    int msg_len = strlen(input) + 1;
    msg = (char*)malloc(msg_len);
    strcpy(msg, input);
    
    // aggiorno la condizione
    pthread_mutex_lock(&mymutex);
    presente = 1;
    pthread_mutex_unlock(&mymutex);
    
    printf("write_message: aggiunto il messaggio alla mailbox\n");
    
    // risveglio eventuali thread in attesa
    printf("write_message: risveglio il thread in attesa\n");
    pthread_cond_signal(&mycond);
    
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) 
{
    pthread_t t1;
    pthread_t t2;
    
    pthread_create(&t1, NULL, read_message, NULL);
    pthread_create(&t2, NULL, write_message, NULL);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    
    return 0;
}


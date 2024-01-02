/* 
 * es_resourcemanager.c
 *
 * Questo programma gestisce un certo numero di risorse finite dello stesso tipo.
 *
 * Il programma crea 3 thread che richiedono di usare n risorse ciascuno. Se
 * non ci sono abbastanza risorse disponibili, il thread si mette in attesa
 * su una variabile condition.
 * Il programma crea, inoltre, 3 thread che aggiungono m risorse ciascuno.
 * Dopo l'aggiunta, il thread risveglia tutti quelli bloccati sulla condition
 * in modo che possano tentare di prelevare le risorse necessarie.
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pthread_mutex_t mymutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t mycond = PTHREAD_COND_INITIALIZER;

// risorsa condivisa
unsigned int resources = 0;

struct st
{
    int index;
    int amount;
};

// il thread si blocca sulla condition se non ci sono risorse disponibili
void* get_resources(void* arg)
{
    struct st pars = *((struct st*)arg);
    int index = pars.index;
    int amount = pars.amount;
    
    pthread_mutex_lock(&mymutex);
    while (amount > resources) 
    {
        printf("(get_resources %d) mi servono %d risorse, disponibili %d. Attendo...\n", index, amount, resources);
        pthread_cond_wait(&mycond, &mymutex);
    }
    resources -= amount;
    printf("(get_resources %d) consumo %d risorse, disponibili %d\n", index, amount, resources);
    pthread_mutex_unlock(&mymutex);
    
    printf("(get_resources %d) termino\n", index);
}

// il thread aggiunge delle risorse e risveglia tutti quelli bloccati
void* add_resources(void* arg)
{
    struct st pars = *((struct st*)arg);
    int index = pars.index;
    int amount = pars.amount;
    
    pthread_mutex_lock(&mymutex);
    resources += (int)amount;
    printf("(add_resources %d) aggiunte %d risorse, disponibili %d\n", index, amount, resources);
    pthread_cond_broadcast(&mycond);
    pthread_mutex_unlock(&mymutex);
}

int main() 
{
    pthread_t gt[3], at[3];
    struct st pars_gt[3], pars_at[3];
    
    unsigned int n = 2;   // risorse richieste
    unsigned int m = 2;   // risorse aggiunte
    unsigned int i;
    
    for (i = 0; i<3; i++)
    {
       pars_at[i].index = i;
       pars_at[i].amount = m;
       pthread_create(&at[i], NULL, add_resources, (void*)&pars_at[i]);
    }
    for (i = 0; i<3; i++)  
    {
       pars_gt[i].index = i;
       pars_gt[i].amount = n;
       pthread_create(&gt[i], NULL, get_resources, (void*)&pars_gt[i]);
    }
    
    for (i = 0; i<3; i++) {
       pthread_join(at[i], NULL);
       pthread_join(gt[i], NULL);
    }
  
    return 0;
}


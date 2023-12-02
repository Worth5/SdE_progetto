/*
 * es_exec.c
 *
 * Questo programma crea un processo figlio, il quale 
 * esegue il programma esterno "somma"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int l(char a1[], char a2[]) {
    // l = lista di argomenti separati
    execl("./somma", "somma", a1, a2, (char*)0);
}

int v(char a1[], char a2[]) {
    // v = array contenente gli argomenti
    char* args[] = {"somma", a1, a2, (char*)0}; 
    execv("./somma", args);
}

int lp(char a1[], char a2[]) {
    // lp = lista di argomenti separati, cerca eseguibile in variabile PATH
    execlp("somma", "somma", a1, a2, (char*)0);
}


int vp(char a1[], char a2[]) {
    // vp = lista di argomenti separati, cerca eseguibile in variabile PATH
    char* args[] = {"somma", a1, a2, (char*)0}; 
    execvp("somma", args);
}

int le(char a1[], char a2[]) {
    // le = lista di argomenti separati, array contenente variabili di ambiente
    char* env[] = {"HOME=/home/studente", "USER=pippo", (char*)0}; 
    execle("./somma", "somma", a1, a2, (char*)0, env);
}

int ve(char a1[], char a2[]) {
    // ve = array contenente gli argomenti, array contenente variabili di ambiente
    char* args[] = {"somma", a1, a2, (char*)0}; 
    char* env[] = {"HOME=/home/studente", "USER=pippo", (char*)0}; 
    execve("./somma", args, env);
}


int main(int argc, char* argv[]) 
{
    pid_t pid;
    int status;
    
    char arg1[4];
    char arg2[4];
    char mode[3];
    
    if (argc != 4)
    {
        fprintf(stderr, "Numero errato di argomenti, attesi %d\n", argc);
        exit(EXIT_FAILURE);
    }       
    else
    {
        strcpy(arg1, argv[1]);  // primo addendo
        strcpy(arg2, argv[2]);  // secondo addendo
        strcpy(mode, argv[3]);  // tipo di exec
    }
    
    // creazione processo figlio
    
    if ((pid = fork()) < 0) 
    {
        fprintf(stderr, "Errore durante la creazione del processo figlio\n");
        exit(EXIT_FAILURE);
    } 
    else if (pid == 0) 
    {
        // usa una diversa versione di exec a seconda dell'argomento
        // passato da linea di comando
        
        if (strcmp(mode, "l") == 0)         l(arg1, arg2);
        else if (strcmp(mode, "v") == 0)    v(arg1, arg2);
        else if (strcmp(mode, "lp") == 0)   lp(arg1, arg2);
        else if (strcmp(mode, "vp") == 0)   vp(arg1, arg2);
        else if (strcmp(mode, "le") == 0)   le(arg1, arg2);
        else if (strcmp(mode, "ve") == 0)   ve(arg1, arg2);
        else
        {
            fprintf(stderr, "Modo %s non riconosciuto\n", mode);
            exit(EXIT_FAILURE);
        }
                
        // si arriva qui solo se la exec fallisce
        printf("Errore nell'invocazione di exec()\n");
        exit(EXIT_FAILURE); 
    } 
    else 
    {
        // attendi il processo figlio e ottieni il suo stato di uscita
        if (wait(&status) < 0) 
        {
            fprintf(stderr, "Errore nella wait\n");
            exit(EXIT_FAILURE);
        }
        
        // controllo dello stato di uscita del processo figlio
        if (status != 0)
            printf("Operazione di somma non riuscita\n");
        else
            printf("Operazione di somma completata\n");

    }
    return EXIT_SUCCESS;
}
 

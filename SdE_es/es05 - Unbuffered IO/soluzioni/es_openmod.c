/* 
 * es_openmod.c
 *
 */

#include <fcntl.h>   // for open() 
#include <stdio.h>   // for printf() and stderr
#include <string.h>  // for strcpy() and strcmp()
#include <unistd.h>  // for close()

#define MAXLEN 32
#define MAXFD 100

// dichiarazione di una file descriptor table personalizzata
struct filedes 
{
    int open;
    int fd;
    char name[MAXLEN];
};
struct filedes fdtable[MAXFD];


int open_mod(const char name[], char c)
{
    int fd, flags;
    
    // converti carattere in flags
    if (c == 'r')
        flags = O_CREAT | O_RDONLY;
    else if (c == 'w')
        flags = O_CREAT | O_WRONLY;
    else if (c == 'a')
        flags = O_CREAT | O_WRONLY | O_APPEND;
    else
        return -1; 
    
    // apri file con flag O_CREAT
    fd = open(name, flags, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
    if (fd == -1)
        return -1;
    
    // aggiungi il nuovo fd alla tabella dei file descriptor personalizzata
    struct filedes newfd;
    newfd.open = 1;
    newfd.fd = fd;
    strcpy(newfd.name, name);
    fdtable[fd] = newfd;
    
    return fd;
}

int close_mod(const char name[])
{
    // cerca il primo file descriptor associato al nome del file passato come argomento
    for (int i=3; i<MAXFD; i++)
    {
        if (fdtable[i].open == 1 && strcmp(fdtable[i].name, name) == 0)
        {
            // trovato: aggiorna tabella e chiudi file descriptor
            fdtable[i].open = 0;
            close(i);
            return i;
        }
    }
    
    // non trovato
    return -1;
}


int main()
{
    int fd;
    
    // inizializza tabella dei file descriptor personalizzata
    // non si considerano i file descriptor relativi agli stream predefiniti
    for (int i=3; i<MAXFD; i++)
        fdtable[i].open = 0;
        
    if ( (fd = open_mod("file3", 'r')) == -1)
        printf ("error in open_mod\n");
    printf("fd %d opened\n", fd);
    
    if ( (fd = open_mod("file4", 'w')) == -1)
        printf ("error in open_mod\n");
    printf("fd %d opened\n", fd);
    
    if ( (fd = open_mod("file5", 'a')) == -1)
        printf ("error in open_mod\n");
    printf("fd %d opened\n", fd);
    
    if ( (fd = close_mod("file4")) == -1)
        printf ("error in close_mod\n");
    printf("fd %d closed\n", fd);
        
    if ( (fd = open_mod("file5", 'a')) == -1)
        printf ("error in open_mod\n");
    printf("fd %d opened\n", fd);
    
    return 0;
    
}

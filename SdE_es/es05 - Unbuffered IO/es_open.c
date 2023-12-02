/*
 * es_open.c 
 * 
 * Questo programma mostra alcuni casi di apertura di file, 
 * utilizzando la syscall open()
 */

#include <fcntl.h>   // open()
#include <errno.h>   // errno
#include <stdio.h>   // fprintf()
#include <stdlib.h>  // exit() e EXIT_FAILURE
#include <string.h>  // strerror()
#include <unistd.h>  // close()



// Funzione che crea un file e lo apre in lettura
// Sul nuovo file vengono assegnati i permessi di lettura e scrittura 
// per l'utente proprietario
void createFile(const char* filename) 
{
    int fd;
    if ((fd = open(filename, O_RDONLY | O_CREAT, S_IRUSR | S_IWUSR)) == -1) 
    {
        // output su STDERR
        fprintf(stderr, "Unable to create new file %s: %s\n", filename, strerror(errno));
        return; 
    }

    printf("%s created. Its file descriptor is: %d\n", filename, fd);

    // Nota: il file descriptor non viene chiuso. 
    //
    // Invocando questa funzione piu' volte, si nota che il numero del file 
    // descriptor incrementa.
}


// Funzione che crea un file e lo apre in lettura
// Se il file esiste gia', restituisce un messaggio di errore a causa del flag O_EXCL
void createFileExcl(const char* filename)
{
    int fd;
    if ((fd = open(filename, O_RDONLY | O_CREAT | O_EXCL,S_IRUSR | S_IWUSR)) == -1) 
    {
        fprintf(stderr, "Unable to create new file %s: %s\n", filename, strerror(errno));
        return;
    }

    // Se il file non e' stato creato, la close() fallisce
    if (close(fd) == -1) {
        fprintf(stderr, "Closing failed: %s\n", strerror(errno));
    }
}


// Funzione che apre un file in lettura
// Se il file non esiste, restituisce un messaggio di errore
void openFile(const char* filename) 
{
    int fd;
    if ((fd = open(filename, O_RDONLY)) == -1)
    {
        fprintf(stderr, "Unable to open file %s: %s\n", filename, strerror(errno));
        return;
    }
     
    printf("%s successfully opened. Its file descriptor is: %d\n", filename, fd);

    close(fd);
    printf("%s closed\n", filename);
}


// Funzione che apre un file (esistente) in lettura e ne imposta la lunghezza a zero byte (flag O_TRUNC)
// Se il file non esiste, restituisce un messaggio di errore
void truncateFile(const char* filename)
{
    int fd;
    if ((fd = open(filename, O_RDONLY | O_TRUNC)) == -1) {
        fprintf(stderr, "Unable to open file %s: %s\n", filename, strerror(errno));
        return;
    }

    printf("%s successfully opened and truncated. Its file descriptor is: %d\n", filename, fd);

    close(fd);
    printf("%s closed\n", filename);
}


int main() 
{
    // la funzione system() esegue il comando shell specificato come argomento
    system("ls -l");  
    
    // crea un file e lo apre in lettura
    const char* filename = "filediesempio";
    printf("\n [1] === Crea file %s === \n",filename);
    createFile(filename);
    system("ls -l"); 
    
    // crea un secondo file e lo apre in lettura
    const char* filename2 = "filediesempio2";
    printf("\n [2] === Crea file %s === \n",filename2);
    createFile(filename2);
    system("ls -l"); 
    
    // crea un file con lo stesso nome
    printf("\n [3] === Crea file %s === \n",filename2);
    createFile(filename2);
    system("ls -l");
    
    // crea un file con lo stesso nome
    printf("\n [4] === Crea file %s (con O_EXCL flag) === \n",filename2);
    createFileExcl(filename2);
    system("ls -l");

    // apre un file esistente in lettura
    printf("\n [5] === Apri file %s in lettura === \n",filename);
    openFile(filename);
    system("ls -l");
    
    // apre un file non esistente in lettura
    const char* filename3 = "nonesiste";
    printf("\n [6] === Apri file %s in lettura === \n",filename3);
    openFile(filename3);
    system("ls -l");
    
    // scrivi dei byte su un file esistente
    printf("\n [6.5] === Scrivi dei byte su filediesempio === \n");
    system("echo ciao > filediesempio");
    system("ls -l");
    
    // apre un file esistente e lo azzera
    printf("\n [7] === Apri file %s in lettura === \n",filename);
    truncateFile(filename);
    system("ls -l");
    
    return 0;
}

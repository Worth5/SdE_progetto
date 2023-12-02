/*
 * es_readwriteint.c
 * 
 * Questo programma mostra che le system call read/write leggono/scrivono dati binari.
 * Il programma prova a leggere un numero intero da un file, lo incrementa e scrive il 
 * risultato su un altro file.
 * Le funzioni con suffisso "_err" mostrano degli esempi di errato utilizzo 
 */


#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int readint_err()
{
    int fd;
    size_t readbytes;
    int n;
    
    // open file for reading
    if ( (fd = open("textfile", O_RDONLY)) < 0) {
        // error
        fprintf(stderr, "Error while opening in read mode. Exit.\n");
        exit(EXIT_FAILURE);
    }
    
    // try to read one integer 
    if ( (readbytes = read(fd, &n, sizeof(int)) < 0) ) {
        // error
        fprintf(stderr, "Error while reading. Exit.\n");
        exit(EXIT_FAILURE);
    } 
   
    printf("I read one integer, n=%d\n",n);
    
    // close file
    close(fd);
    
    return n;
}

int readint()
{
    int fd;
    size_t readbytes;
    const int NUMLEN = 4;  // assume that the integer to read has 4 digits
    char buffer[NUMLEN+1]; // add one byte for the '\0' character
    int n;
     
    // open file for reading
    if ( (fd = open("textfile", O_RDONLY)) < 0) {
        // error
        fprintf(stderr, "Error while opening in read mode. Exit.\n");
        exit(EXIT_FAILURE);
    }
    
    // try to read one integer 
    if ( (readbytes = read(fd, buffer, NUMLEN) < 0) ) {
        // error
        fprintf(stderr, "Error while reading. Exit.\n");
        exit(EXIT_FAILURE);
    } 
    buffer[NUMLEN] = '\0';
   
    // convert string to int 
    n = atoi(buffer);
    
    printf("I read one integer, n=%d\n",n);
    
    // close file
    close(fd);
    
    return n;
}

void writeint_err(int n)
{
    int fd;
    size_t writtenbytes;
    
    // create file and open it for writing
    if ( (fd = open("textfile_incr_err", O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR)) < 0) {
        // error
        fprintf(stderr, "Error while opening in write mode. Exit.\n");
        exit(EXIT_FAILURE);
    }
    
    // write an integer to the file
    if ( (writtenbytes = write(fd, &n, sizeof(int))) != sizeof(int)) {
        // error
        fprintf(stderr, "Error while writing. Exit.\n");
        exit(EXIT_FAILURE);
    }
       
    // close file
    close(fd);
}

void writeint(int n)
{
    int fd;
    size_t writtenbytes;
    const int NUMLEN = 4;  // assume that the integer to read has 4 digits
    char buffer[NUMLEN];  
    
    // create file and open it for writing
    if ( (fd = open("textfile_incr", O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR)) < 0) {
        // error
        fprintf(stderr, "Error while opening in write mode. Exit.\n");
        exit(EXIT_FAILURE);
    }
    
    // convert int to string 
    sprintf(buffer, "%d", n);
   
    // write an integer to the file
    if ( (writtenbytes = write(fd, buffer, NUMLEN)) != sizeof(int)) {
        // error
        fprintf(stderr, "Error while writing. Exit.\n");
        exit(EXIT_FAILURE);
    }
       
    // close file
    close(fd);
}



int main()
{
    int n;
    n = readint_err();
    n = readint();
    writeint_err(n+1);
    writeint(n+1);

    return 0;
}

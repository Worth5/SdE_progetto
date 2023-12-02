/*
 * es_copy.c
 * 
 * Questo programma legge da standard input e stampa su standard output.
 * Testare il tempo impiegato per l'esecuzione con il comando 'time', usando
 * un file di input piuttosto grande. Per esempio, per generare un file da 
 * 600MB usare il comando "dd if=/dev/zero of=bigfile bs=600M count=1"
 * 
 *     time ./es_copy < bigfile > /dev/null
 * 
 * Eseguire il test con diverse dimensioni del buffer (BUFF_LEN) e 
 * confrontare i risultati
 */

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

int main() {
    int fd, n;
    int BUFF_LEN = 512;
    char buff[BUFF_LEN];

    // legge standard input fino alla fine
    while((n = read(STDIN_FILENO, buff, BUFF_LEN)) > 0) 
    {
        // scrive i byte letti su standard output
        if (write(STDOUT_FILENO, buff, n) != n) {
            fprintf(stdout,"ERROR while writing: %s\n",strerror(errno)); 
            exit(EXIT_FAILURE);
        }
    }
    
    return 0;
}

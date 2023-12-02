/*
 * sortint.c
 * 
 * Questo programma legge un file contenente i primi 100 numeri interi (da 1 a 100),
 * li inserisce (in ordine crescente) in un vettore di interi e li stampa
 * su standard output.
 * Usare unbuffered I/O per leggere da stdin e scrivere su stdout
 * 
 * Suggerimento: per generare un file con 100 numeri in ordine casuale, usare il comando
 * 
 *    seq 100 | shuf > primi100numeri.txt
 */

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>   // per errno
#include <string.h>  // per strerror()
#include <stdio.h>   // per fprintf()
#include <stdlib.h>  // per exit()

int main () {

    const int BUFF_SIZE = 1024;   // dimensione del buffer
    char buff[BUFF_SIZE];         // buffer di lettura
    int byte_letti;               // variabile per il numero di byte letti con la read() 
    
    char c;                       // prossimo carattere letto dal buffer
    const int NUM_LEN = 3;        // numero massimo di cifre per ogni numero
    char str[NUM_LEN+1];          // stringa per il prossimo numero estratto dal buffer (+1 per '\0')
    int j = 0;                    // indice per la posizione nell'array str       
    
    int numero_letto;             // prossimo numero letto dal buffer
    int vett_ord[100];            // vettore che conterra' i numeri letti in ordine crescente

    
    char out[NUM_LEN+1];          // stringa di appoggio per la stampa del numero su stdout
    int byte_scritti;                    
    
    // lettura di BUFF_SIZE byte da stdin
    while ( (byte_letti = read(STDIN_FILENO, buff, BUFF_SIZE)) > 0) {
        
        // per ogni byte letto (potrebbero essere meno di BUFF_SIZE)
        for (int i=0; i<byte_letti; i++) {
            
            c = buff[i];   // considera il prossimo carattere nel buffer
            
            if (c >= '0' && c <= '9') {
                // se il carattere e' una cifra, si aggiunge alla stringa str
                str[j++] = c;
            }
            else {
                // altrimenti, si aggiunge un carattere '\0' alla stringa str
                str[j++] = '\0';
                
                // conversione della stringa in numero intero e inserimento nel vettore
                numero_letto = atoi(str);
                vett_ord[numero_letto-1] = numero_letto;
                
                // reset dell'indice
                j = 0;
            }          
        }
    }
    
    // si scorrono tutti gli elementi del vettore
    for (int i = 0; i<100; i++) {
        
        // conversione del numero intero in stringa prima di fare l'output
        byte_scritti = sprintf(out, "%d", vett_ord[i]);
        
        // aggiunta del carattere di fine linea dopo l'ultima cifra del numero
        out[byte_scritti] = '\n';
        
        // scrittura su stdout
        if (write(STDOUT_FILENO, &out, byte_scritti+1) != byte_scritti+1) {
            fprintf(stderr, "Error in write(): %s", strerror(errno)); 
            exit(EXIT_FAILURE);
        }    
        
    }
                
    return 0;    
}

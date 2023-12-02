/*
 * es_readmsg.c
 * 
 * Questo programma legge una linea da stdin e la scrive su stdout
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() 
{    
    const int BUFFLEN = 1024;
    char buff[BUFFLEN];
    
    // lettura da stdin
    fgets(buff, BUFFLEN, stdin);
        
    // scrittura su stdout della stringa letta da stdin
    printf("[pid=%d] Letta linea: %s\n",getpid(), buff);
        
    return EXIT_SUCCESS; 
} 



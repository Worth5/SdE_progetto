/*
 * multiprocess_server.c
 *
 * Il programma server:
 * - apre un socket di tipo stream
 * - attende la connessione da parte di un client 
 * - per ogni connessione, crea un processo figlio che, ciclicamente,
 *   riceve un messaggio dal client e invia una risposta. Quando viene
 *   ricevuta la stringa 'exit', il processo figlio termina
 * - il processo genitore si mette in attesa di una nuova connessione
 *
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void stream_socket(int sd)
{
    const int MAXQUEUE = 10;
    
    int conn_sd;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);;
    char addr_str[INET_ADDRSTRLEN];
    
    const int BUFFLEN = 1024;
    char buff[BUFFLEN];
    ssize_t rcvd_bytes, snd_bytes;
    
    pid_t pid;
    
    // --- LISTENING --- //
    if (listen(sd, MAXQUEUE) < 0)
    {
        fprintf(stderr, "Impossibile mettersi in attesa su socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    while (1)
    {    
        // --- ATTESA DI CONNESSIONE --- //
        printf("--- In attesa di connessione ---\n");
        if ( (conn_sd = accept(sd, (struct sockaddr*)&client_addr, &client_addr_len)) < 0)
        {
            fprintf(stderr, "Impossibile accettare connessione su socket: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }  
        
        if ((pid = fork()) < 0)
        {
            fprintf(stderr, "Impossibile creare un nuovo processo: %s\n", strerror(errno));
        }
        else if (pid == 0)
        {
            
            // conversione dell'indirizzo in formato numerico
            if (inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, addr_str, INET_ADDRSTRLEN) < 0)
            {
                fprintf(stderr, "Impossibile convertire l'indirizzo\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            printf("Connesso col client %s:%d\n", addr_str, ntohs(client_addr.sin_port));
            
            while(1)
            {
                // --- RICEZIONE DATI --- //
                if ( (rcvd_bytes = recv(conn_sd, &buff, BUFFLEN, 0)) < 0)
                {
                    fprintf(stderr, "Impossibile ricevere dati su socket: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                buff[rcvd_bytes] = '\0';
        
                printf("Ricevuto '%s'\n", buff);
        
                // --- INVIO RISPOSTA --- //
                if ( (snd_bytes = send(conn_sd, "OK", 2, 0)) < 0)
                {
                    fprintf(stderr, "Impossibile inviare dati su socket: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                printf("Risposta inviata\n");
        
                if (strcmp(buff, "exit") == 0)
                    break;
            }
            
            // --- CHIUSURA SOCKET CONNESSO --- //
            close(conn_sd);
            
            // termina il processo figlio
            exit(EXIT_SUCCESS);
            
        }
    }
}

int main(int argc, char* argv[])
{
    int sd;
    int type;
    struct sockaddr_in sa;
    in_addr_t address;
    char* addr_str = "127.0.0.1";
    int port_no = 10000;
    int reuse_opt = 1;
    
    
    // --- CREAZIONE SOCKET --- //
    if ( (sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        fprintf(stderr, "Impossibile creare il socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } 

    // --- BINDING --- //
    
    // conversione dell'indirizzo in formato numerico
    if (inet_pton(AF_INET, addr_str, (void*)&address) < 0)
    {
        fprintf(stderr, "Impossibile convertire l'indirizzo: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // preparazione della struttura contenente indirizzo IP e porta
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port_no);
    sa.sin_addr.s_addr = address;

    // imposta opzione per riusare l'indirizzo e porta
    if (type == SOCK_STREAM)
        setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &reuse_opt, sizeof(int));

    // associazione indirizzo a socket
    if (bind(sd, (struct sockaddr*)&sa, sizeof(sa)) < 0)
    {
        fprintf(stderr, "Impossibile associare l'indirizzo a un socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("Socket %d associato a %s:%d\n", sd, addr_str, port_no);

    stream_socket(sd);
    
    // --- CHIUSURA SOCKET --- //
    close(sd);
        
    return EXIT_SUCCESS;
}

/*
 * server_v0
 *
 * Il programma server:
 * - apre un socket di tipo connection-oriented
 * - si mette in attesa di una connessione da un client
 * - attende un messaggio dal client
 * - chiude il socket
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char* argv[])
{
    char* addr_str = "127.0.0.1";
    int port_no = 10000;
    
    // --- CREAZIONE SOCKET --- //
    int sd;
    if ( (sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        fprintf(stderr, "Impossibile creare il socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } 

//    // for debugging
//    int reuse_opt = 1;
//    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &reuse_opt, sizeof(int));
        
    // --- BINDING --- //
    
    // conversione dell'indirizzo in formato numerico
    in_addr_t address;
    if (inet_pton(AF_INET, addr_str, (void*)&address) < 0)
    {
        fprintf(stderr, "Impossibile convertire l'indirizzo: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // preparazione della struttura contenente indirizzo IP e porta
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port_no);
    sa.sin_addr.s_addr = address;

    // associazione indirizzo a socket
    if (bind(sd, (struct sockaddr*)&sa, sizeof(sa)) < 0)
    {
        fprintf(stderr, "Impossibile associare l'indirizzo a un socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("Socket %d associato a %s:%d\n", sd, addr_str, port_no);
    
    // --- LISTENING --- //
    // massimo 10 richieste di connessione pendenti
    if (listen(sd, 10) < 0)
    {
        fprintf(stderr, "Impossibile mettersi in attesa su socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
        
    // --- ATTESA DI CONNESSIONE --- //
    printf("--- In attesa di connessione ---\n");
    
    int conn_sd;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    if ( (conn_sd = accept(sd, (struct sockaddr*)&client_addr, &client_addr_len)) < 0)
    {
        fprintf(stderr, "Impossibile accettare connessione su socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } 
    
    // conversione dell'indirizzo in formato numerico
    char client_addr_str[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_addr_str, INET_ADDRSTRLEN) < 0)
    {
        fprintf(stderr, "Impossibile convertire l'indirizzo\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("Connesso col client %s:%d\n", client_addr_str, ntohs(client_addr.sin_port));
        
    // --- RICEZIONE DATI --- //
    const int BUFFLEN = 1024;
    char buff[BUFFLEN];
    ssize_t rcvd_bytes;
    if ( (rcvd_bytes = recv(conn_sd, &buff, BUFFLEN, 0)) < 0)
    {
        fprintf(stderr, "Impossibile ricevere dati su socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    buff[rcvd_bytes] = '\0';
        
    printf("Ricevuto '%s'\n", buff);
        
    // --- CHIUSURA SOCKET CONNESSO --- //
    close(conn_sd);
    
    // --- CHIUSURA SOCKET DI ASCOLTO --- //
    close(sd);
    
    return EXIT_SUCCESS;
}

/*
 * server_v0
 *
 * Il programma server:
 * - apre un socket di tipo connectionless
 * - attende un messaggio da un client
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
    if ( (sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        fprintf(stderr, "Impossibile creare il socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } 

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

    printf("Socket %d in ascolto su %s:%d\n", sd, addr_str, port_no);
    
    // --- RICEZIONE DATI --- //
    printf("--- In attesa di un messaggio ---\n");
    
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    
    const int BUFFLEN = 1024;
    char buff[BUFFLEN];
    
    ssize_t rb = recvfrom(sd, &buff, BUFFLEN, 0, (struct sockaddr*)&client_addr, &client_addr_len);
    if ( rb < 0)
    {
        fprintf(stderr, "Impossibile ricevere dati su socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    buff[rb] = '\0';
    
    // conversione dell'indirizzo in formato numerico
    char client_addr_str[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_addr_str, INET_ADDRSTRLEN) < 0)
    {
        fprintf(stderr, "Impossibile convertire l'indirizzo\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    printf("Ricevuto '%s' dal client %s:%d\n", buff, addr_str, ntohs(client_addr.sin_port));
   
    // --- CHIUSURA SOCKET --- //
    close(sd);
    
    return EXIT_SUCCESS;
}

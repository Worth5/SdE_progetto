/*
 * client_v1
 *
 * Il programma client:
 * - prende una stringa come argomento 
 * - apre un socket di tipo connection-oriented
 * - invia la lunghezza della stringa al server
 * - invia la stringa al server
 * - attende un messaggio di riscontro dal server
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
    
    if (argc != 2)
    {
        fprintf(stderr, "Il programma richiede un argomento, passati %d\n", argc-1);
        exit(EXIT_FAILURE);
    }
    
    // --- CREAZIONE SOCKET --- //
    int sd;
    if ( (sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        fprintf(stderr, "Impossibile creare il socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } 
    
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
    
    // --- CONNESSIONE --- //
    printf("Connessione...\n");
    if (connect(sd, (struct sockaddr*)&sa, sizeof(struct sockaddr_in)) < 0)
    {
        fprintf(stderr, "Impossibile connettersi: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    ssize_t snd_bytes;
    
    // --- INVIO LUNGHEZZA DEL MESSAGGIO --- //
    int msg_len = htonl( strlen(argv[1]) );   // conversione a formato network
    snd_bytes = send(sd, &msg_len, sizeof(int), 0);
    if (snd_bytes < 0)
    {
        fprintf(stderr, "Impossibile inviare dati: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("Inviati %d bytes\n", snd_bytes);
    
    // --- INVIO DATI --- //
    snd_bytes = send(sd, argv[1], strlen(argv[1]), 0);
    if (snd_bytes < 0)
    {
        fprintf(stderr, "Impossibile inviare dati: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("Inviati %d bytes\n", snd_bytes);
    
    // --- RICEZIONE RISPOSTA --- //
    char resp[3];
    size_t rcvd_bytes = recv(sd, &resp, 2, 0);
    if (rcvd_bytes < 0)
    {
        fprintf(stderr, "Impossibile ricevere dati su socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    resp[rcvd_bytes] = '\0';
    
    if (strcmp(resp, "OK") == 0)
        printf("Ricevuta risposta: OK\n");
    else
        printf("Ricevuta risposta: NOT OK\n");
       
    // --- CHIUSURA SOCKET --- //
    close(sd);
    
    return EXIT_SUCCESS;
}

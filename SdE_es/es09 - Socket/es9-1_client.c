/*
 * client_v3
 *
 * Il programma client:
 * - apre un socket di tipo stream
 * - ciclicamente: 
 * --- legge una stringa da stdin
 * --- la invia al server
 * --- attende la risposta dal server
 * - quando da stdin viene letta la stringa 'exit' termina
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>


void socket_stream(int sd, struct sockaddr_in sa)
{
    const int MAXLEN = 1024;
    char buff[MAXLEN];
    int buff_len;
    char resp[3];
    ssize_t snd_bytes, rcvd_bytes;
    
    // --- CONNESSIONE --- //
    printf("Connessione...\n");
    if (connect(sd, (struct sockaddr*)&sa, sizeof(struct sockaddr_in)) < 0)
    {
        fprintf(stderr, "Impossibile connettersi: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    while(1)
    {
        // --- INVIO DATI --- //
        fgets(buff, MAXLEN, stdin);
        buff_len = strlen(buff);
        if (buff[buff_len - 1] == '\n')
            buff[buff_len - 1] = '\0';
        if ( (snd_bytes = send(sd, buff, strlen(buff), 0)) < 0)
        {    
            fprintf(stderr, "Impossibile inviare dati: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        printf("Inviati %d bytes\n", snd_bytes);
    
        // --- RICEZIONE RISPOSTA --- //
        if ( (rcvd_bytes = recv(sd, &resp, 2, 0)) < 0)
        {
            fprintf(stderr, "Impossibile ricevere dati su socket: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        resp[rcvd_bytes] = '\0';
        
        printf("Ricevuta risposta '%s'\n", resp);
        
        if (strcmp(buff, "exit") == 0)
            break;
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
    
    // --- CREAZIONE SOCKET --- //
    if ( (sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        fprintf(stderr, "Impossibile creare il socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } 
    
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
    
    socket_stream(sd, sa);
        
    // --- CHIUSURA SOCKET --- //
    close(sd);
    
    return EXIT_SUCCESS;
}

/*
 * client_v2
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
    
    const int MAXLEN = 1024;
    char buff[MAXLEN];
    ssize_t snd_bytes, rcvd_bytes;
    while(1)
    {
        // --- LETTURA LINEA DA STDIN --- //
        printf("Inserisci un messaggio da inviare al server: ");
        fgets(buff, MAXLEN, stdin);
        int len = strlen(buff);
        if (buff[len - 1] == '\n')
            buff[len - 1] = '\0';
        
        // --- INVIO LUNGHEZZA DEL MESSAGGIO --- //
        int msg_len = htonl(len-1);   // conversione a formato network
        snd_bytes = send(sd, &msg_len, sizeof(int), 0);
        if (snd_bytes < 0)
        {
            fprintf(stderr, "Impossibile inviare dati: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        printf("Inviati %d bytes\n", snd_bytes);
    
        // --- INVIO MESSAGGIO --- //
        if ( (snd_bytes = send(sd, buff, len-1, 0)) < 0)
        {    
            fprintf(stderr, "Impossibile inviare dati: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        printf("Inviati %d bytes\n", snd_bytes);
    
        // --- RICEZIONE RISPOSTA --- //
        char resp[3];
        rcvd_bytes = recv(sd, &resp, 2, 0);
        if (rcvd_bytes < 0)
        {
            fprintf(stderr, "Impossibile ricevere dati su socket: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        resp[rcvd_bytes] = '\0';
        
        printf("Ricevuta risposta '%s'\n", resp);
        
        if (strcmp(buff, "exit") == 0)
            break;
    }

    // --- CHIUSURA SOCKET --- //
    close(sd);
    
    return EXIT_SUCCESS;
}

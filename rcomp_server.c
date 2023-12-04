//questa è la copia dell esempio prof pari pari se compilata ed eseguita permette di connetterci il client per testarlo
#include <stdio.h>	//for standard I/O
#include <string.h>	//for strerror()
#include <errno.h>	//for errno
#include <fcntl.h>	//for flags like O_RDONLY and SEEK_END
#include <stdlib.h>	//for exit() and EXIT_FAILURE
#include <unistd.h>	//for unbuffered functions-> read(), wriqte(), STDIN_FILENO

#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

int process_client(int conn_sd)
{
    ssize_t rcvd_bytes, snd_bytes;
    while(1)
	{
		// --- RICEZIONE LUNGHEZZA DELLA STRINGA --- //
		int n;
		rcvd_bytes = recv(conn_sd, &n, sizeof(int), 0);
		if (rcvd_bytes < 0)
		{
			fprintf(stderr, "Impossibile ricevere dati su socket: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		n = ntohl(n); // conversione a formato host
		printf("Ricevuto '%d'\n", n);

		char* buff = malloc(n+1);
	
	    // --- RICEZIONE DATI --- //
	    if ( (rcvd_bytes = recv(conn_sd, buff, n, 0)) < 0)
	    {
	        fprintf(stderr, "Impossibile ricevere dati su socket: %s\n", strerror(errno));
	        exit(EXIT_FAILURE);
	    }
	    buff[rcvd_bytes] = '\0';

	    printf("Ricevuto '%s'\n", rcvd_bytes, buff);

	    // --- INVIO RISPOSTA --- //
	    if ( (snd_bytes = send(conn_sd, "OK", 2, 0)) < 0)
	    {
	        fprintf(stderr, "Impossibile inviare dati su socket: %s\n", strerror(errno));
	        exit(EXIT_FAILURE);
	    }
	    printf("Risposta inviata\n");

	    if (strcmp(buff, "exit") == 0)
	        break;
	        
	    free(buff);
	}
	
	// --- CHIUSURA SOCKET CONNESSO --- //
    close(conn_sd);
}

int main(int argc, char* argv[])
{
    char* addr_str = "127.0.0.1";
    int port_no = 1234;

    // --- CREAZIONE SOCKET --- //
    int sd;
    if ( (sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
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

    // DEBUG
    int reuse_opt = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &reuse_opt, sizeof(int));

    // associazione indirizzo a socket
    if (bind(sd, (struct sockaddr*)&sa, sizeof(sa)) < 0)
    {
        fprintf(stderr, "Impossibile associare l'indirizzo a un socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("Socket %d associato a %s:%d\n", sd, addr_str, port_no);
    
    // --- LISTENING --- //
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
	printf("Connesso col client %s:%d\n", addr_str, ntohs(client_addr.sin_port));
	
	pid_t pid;
	if ((pid = fork()) < 0)
	{
	    fprintf(stderr, "Impossibile creare un processo figlio\n", strerror(errno));
	    exit(EXIT_FAILURE);
	}
	else if (pid == 0)
	{
	    // processo figlio
	    process_client(conn_sd);
	    exit(EXIT_SUCCESS);
	}
	else
	{
	    // processo genitore
	    if (wait(NULL) < 0)
	    {
	        fprintf(stderr, "Impossibile creare un processo figlio\n", strerror(errno));
	        exit(EXIT_FAILURE);
	    }
	}
    
    // --- CHIUSURA SOCKET --- //
    close(sd);
        
    return EXIT_SUCCESS;
}






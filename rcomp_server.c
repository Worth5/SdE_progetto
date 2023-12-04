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

int process_client(int conn_sd);

int main(int argc, char* argv[])
{
    char* addr_str = "127.0.0.1";
    int port_no = 1234;

	if(argc == 3){
        if(strchr(argv[1], '.') != NULL){
            addr_str = argv[1];
            port_no = atoi(argv[2]); 	//manca controllo errore.... meglio cmbiare funzione completamente esempio strtol()
        }
        else {
        	addr_str = argv[2];
            port_no = atoi(argv[1]); 	//manca controllo errore.... meglio cmbiare funzione completamente
        }
	}
    else if(argc == 2){
        if(strchr(argv[1], '.') != NULL){
			addr_str = argv[1];
		}
		else{
			port_no = atoi(argv[1]);
		}
	}


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



int process_client(int conn_sd)
{
    ssize_t rcvd_bytes, snd_bytes;
    while(1)
	{
		// --- RICEZIONE LUNGHEZZA DELLA STRINGA --- //
		char c;
		rcvd_bytes = recv(conn_sd, &c, sizeof(char), 0);
		printf("%c",c);
	}
	
	// --- CHIUSURA SOCKET CONNESSO --- //
    close(conn_sd);
}

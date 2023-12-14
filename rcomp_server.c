//questa è la copia dell esempio prof pari pari se compilata ed eseguita permette di connetterci il client per testarlo
#include <stdio.h>		//for standard I/O
#include <string.h>		//for strerror()
#include <errno.h>		//for errno
#include <fcntl.h>		//for flags like O_RDONLY and SEEK_END
#include <stdlib.h>		//for exit() and EXIT_FAILURE
#include <unistd.h>		//for unbuffered functions-> read(), wriqte(), STDIN_FILENO

#include <sys/socket.h>	//for socket
#include <arpa/inet.h>	//for inet_pton() INADDR_ANY credo...
#include <sys/wait.h>	//boh ahah non ricordo perche l'ho inserito..... 🥲

int setup(int argc, char* argv[]);
int connect_client(int argc, char* argv[]);
void add(int conn_sd);
void compress(int conn_sd);

int main(int argc, char* argv[]){

	struct request rq = struct request {"","",0};
	int sd = setup(int argc, char* argv[]);	//crea socket imposta sockopt esegue bind e listen 

	while(1){
		int conn_sd = connect_client(sd); // esegue connet e restituisce connection socket
		int valid_input = 1;
		char command[10] = "0";

		while (valid_input && strcmp(command, "q")){ // finche input è valido e diverso da "q"
			ssize_t rcvd_bytes;

			if ((rcvd_bytes = recv(conn_sd, &command, sizeof(command), 0)) < 0){
				fprintf(stderr, "Impossibile ricevere dati su socket: %s\n", strerror(errno));
				exit(EXIT_FAILURE);
			}
			else if(!strcmp(command,"add")){
				add(conn_sd);
			}
			else if((!strcmp(command,"compj")) || (!strcmp(command,"compz"))){
				compress(conn_sd);
			}
			else if(!strcmp(command,"q")){
				printf("Connection interrupted by client\n")
			}
			else{
				valid_input = 0;
				printf("Invalid command detected\n");
			}
		}
		printf("Closing connection\n")
		close(conn_sd);
	}

	
    return EXIT_SUCCESS;
}

int setup(int argc, char *argv[]){ // create socket + socket opt + bind() return socket
	// char* addr_str = "127.0.0.1";
	int port_no = 1234;

	if (argc == 2){
		int temp = strtol(argv[1]);
		if ((temp <= 65535) && (temp >= 0))
			port_no = argv[1];
		else
			printf("Set default port = %s\n", port_no);
	}

	int sd;
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		fprintf(stderr, "Impossibile creare il socket: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	in_addr_t address;
	if (inet_pton(AF_INET, addr_str, (void *)&address) < 0){ // conversione dell'indirizzo in formato numerico
		fprintf(stderr, "Impossibile convertire l'indirizzo: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in sa; // preparazione della struttura contenente indirizzo IP e porta
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port_no);
	sa.sin_addr.s_addr = address;

	int reuse_opt = 1;
	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &reuse_opt, sizeof(int));

	// --- BIND --- //
	if (bind(sd, (struct sockaddr *)&sa, sizeof(sa)) < 0){ // associazione indirizzo a socket
		fprintf(stderr, "Impossibile associare l'indirizzo a un socket: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	printf("Socket %d associato a %s:%d\n", sd, addr_str, port_no);

	// --- LISTENING --- //
	if (listen(sd, 10) < 0){
		fprintf(stderr, "Impossibile mettersi in attesa su socket: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	return sd;
}

int connect_client(int sd){
	
	// --- ATTESA DI CONNESSIONE --- //
	printf("--- In attesa di connessione ---\n");
	
	int conn_sd;
	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(client_addr);

	if ( (conn_sd = accept(sd, (struct sockaddr*)&client_addr, &client_addr_len)) < 0){
	    fprintf(stderr, "Impossibile accettare connessione su socket: %s\n", strerror(errno));
	    exit(EXIT_FAILURE);
	}  
	
	// conversione dell'indirizzo in formato numerico
	char client_addr_str[INET_ADDRSTRLEN];
	if (inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_addr_str, INET_ADDRSTRLEN) < 0){
	    fprintf(stderr, "Impossibile convertire l'indirizzo\n", strerror(errno));
	    exit(EXIT_FAILURE);
	}

	printf("Connesso al client %s:%d\n", addr_str, ntohs(client_addr.sin_port));
	return conn_sd;
}

void get_request(conn_sd, struct request &rq){
	ssize_t rcvd_bytes, snd_bytes;
	char c;
	rcvd_bytes = recv(conn_sd, &c, sizeof(char), 0);
	printf("%c",c);
	if (rq.valid = check_valid(c)){
		if(c=='a'){
			add();
		}else if(( c=='j') || (c =='z')){
			compress();
		}else if(c=='q'){
			quit();
		}else 
			
	}
}










int process_client(int conn_sd)//non usata
{
    ssize_t rcvd_bytes, snd_bytes;
    while(1)
	{
		// --- RICEZIONE LUNGHEZZA DELLA STRINGA --- //
		
	}
	
	// --- CHIUSURA SOCKET CONNESSO --- //
    close(conn_sd);
}

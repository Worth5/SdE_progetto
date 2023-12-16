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


//funzioni main
int setup(int argc, char* argv[]);
int connect_client(int sd);

//funzioni server richieste
void add(int conn_sd);
void compress(int conn_sd);

//funzioni subordinate
int parse_argv_for_port(int argc, char* argv[]);



int main(int argc, char* argv[]){

	int sd = setup(argc, argv);	//crea socket imposta sockopt esegue bind e listen 

	while(1){
		int conn_sd = connect_client(sd); // esegue connet e restituisce connection socket
		int valid_input = 1;
		char command[10] = "0";

		while (valid_input && strcmp(command, "q")){ // finche input è valido e diverso da "q"

			ssize_t rcvd_bytes= recv(conn_sd, &command, 1, 0);

			if (rcvd_bytes < 0){
				fprintf(stderr, "Impossibile ricevere dati su socket: %s\n", strerror(errno));
				exit(EXIT_FAILURE);
			}
			else if(!strcmp(command,"a")){
				add(conn_sd);//riceve file e lo salva
			}
			else if((!strcmp(command,"j")) || (!strcmp(command,"z"))){
				compress(conn_sd);//fork al comando tar con exec poi manda il file al client
			}
			else if(!strcmp(command,"q")){
				printf("Connection interrupted by client\n");
			}
			else{
				valid_input = 0;
				printf("Invalid command detected\n");
			}
		}
		printf("Closing connection\n");
		close(conn_sd);
	}
	close(sd);
    return EXIT_SUCCESS;
}


int setup(int argc, char *argv[]){ // create socket + socket opt + bind() return socket

	int port_no = 1234;
	int port_pos = parse_argv_for_port(argc, argv);
	if (port_pos > 0){
		port_no = atoi(argv[port_pos]);
		printf("port:%d\n", port_no);
	}
	else{
		printf("no valid port, using default:%d\n", port_no);
	}

	int sd;
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		fprintf(stderr, "Impossibile creare il socket: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	char *addr_str="127.0.0.1";
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


int parse_argv_for_port(int argc, char* argv[]){

	//debug("parse_argv_for_port()\n",3);
	for(int j = 1; j < argc; j++){
		//debug("arg\n",4);
		int check=1;
		for(int i = 0; argv[j][i]!='\0'; i++){
			char c[3]={i+'0','\n','\0'};
			//debug(c,5);
			if(argv[j][i]<'0' || argv[j][i]>'9'){
				check=0;
			}
		}
		if(check){
			//debug("valid port found\n",3);
			return j;
		}
		
	}
	//debug("no arg has valid port\n",3);
	return -1;
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

	printf("Connesso al client %s:%d\n", client_addr_str, ntohs(client_addr.sin_port));
	return conn_sd;
}

void add(int conn_sd){
	printf("add");
}
void compress(int conn_sd){
	printf("compress");
}
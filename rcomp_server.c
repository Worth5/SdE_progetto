//questa è la copia dell esempio prof pari pari se compilata ed eseguita permette di connetterci il client per testarlo
#include <stdio.h>		//for standard I/O
#include <string.h>		//for strerror()
#include <errno.h>		//for errno
#include <fcntl.h>		//for flags like O_RDONLY and SEEK_END
#include <stdlib.h>		//for exit() and EXIT_FAILURE
#include <unistd.h>		//for unbuffered functions-> read(), wriqte(), STDIN_FILENO

#include <sys/socket.h>	//for socket
#include <arpa/inet.h>	//for inet_pton() INADDR_ANY credo...


#define BUFFSIZE 4096

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
	printf("Add request received\n");

	//creo nome dir univoco aggiungendo pid
	char *dir_name = srtcat("rcomp_server.temp.",itoa(getpid()));
	

	//mkdir se dir non esiste
	struct stat dir_info;
	mode_t mode = S_IRUSR | S_IWUSR | S_IXUSR;
	if(stat(dir_name, &dirInfo)==-1){
		if(mkdir(dir_name, mode)<0){
			fprintf(stderr, "Error creating directory: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		
	}
	//sposto working directory in dir_name
	chdir(dir_name);

	//ricevi lunghezza nome del file
	int file_name_len;
    if (recv(conn_sd, &file_name_len, sizeof(int), 0) < 0) {
        fprintf(stderr, "Error receiving size of file name: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    int file_name_len= ntohl(file_name_len);

	//alloco spazio in memoria e ricevo stringa nome del file
	char *filename = malloc(file_name_len);
    if (recv(conn_sd, &filename, file_name_len, 0) < 0) {
        fprintf(stderr, "Error receiving file name: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

	//check file gìà presente in questo caso non si fa niente e verrà sovrascritto
	struct stat file_info;
	if(stat(filename, &file_info) == 0){
		Printf("File alreaddy exists, overwriting with new file\n");
	}

	//crea file all'interno della dir con nome ricevuto
 	int fd;
	if((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0777)) < 0){
		fprintf(stderr, "ERROR: cannot create file %s (%s)\n", filename, strerror(errno));
		exit(EXIT_FAILURE);
	}

	//ricevi grandezza file
	int file_size;
    ssize_t rcvd_bytes = recv(conn_sd, &file_size, sizeof(int), 0);
    if (rcvd_bytes < 0) {
        fprintf(stderr, "Error receiving file size: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    int file_size = ntohl(file_size);
	
	//ciclo recv from socket, write on file
	const int BUFFSIZE = 4096;
    char buff[BUFFSIZE];
    int remaining_bytes = file_size;

    while (remaining_bytes > 0) {
         
        if ((rcvd_bytes = recv(conn_sd, buff, sizeof(buff), 0)) < 0) {
            fprintf(stderr, "Error receiving file data: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (write(fd, buff, rcvd_bytes) < 0) {
            fprintf(stderr, "Error writing to file: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        remaining_bytes -= rcvd_bytes;
    }

    printf("File received and saved as: %s\n", filename);

    // Close the file descriptor
    close(fd);
	chdir("..");//gestione errore?
}

void compress(int conn_sd) {
    printf("Compress request received\n");

    char dir_name[50];
    sprintf(dir_name, "rcomp_server.temp.%d", getpid());

    // receive compression type from client
	char arg[2];
    ssize_t rcvd_bytes;
    if ((rcvd_bytes = recv(conn_sd, arg, sizeof(arg), 0)) < 0) {
        fprintf(stderr, "Error receiving compression type: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
	//setup tar command
    char tar_command[100];
    if (strcmp(arg, "z") == 0) {
        snprintf(tar_command, sizeof(tar_command), "tar czf - -C ./%s .", dir_name);
    } else if (strcmp(arg, "j") == 0) {
        snprintf(tar_command, sizeof(tar_command), "tar cjf - -C ./%s .", dir_name);
    } else {
        fprintf(stderr, "Invalid compression type received: %s\n", arg);
        exit(EXIT_FAILURE);
    }

    // controllo che la cartella esista
	struct stat directory_stat;
    if (stat(dir_name, &dir_stat) == -1) {
		//dir non esiste -> nessun file da comprimere
        if (errno == ENOENT) {//ENOENT
	        char *str = "no";
            if (send(conn_sd, str, strlen(str) + 1, 0) < 0) {
                fprintf(stderr, "Error sending data: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
			printf("No file ready for compression");
			return;
        }
		else{//errore diverso in lettura dir
			fprintf(stderr, "ERROR: cannot open dir %s (%s)\n", filename, strerror(errno));
			exit(EXIT_FAILURE);
		}
    } 

	//ok to server comincia compressione
	char *str = "ok";
	if (send(conn_sd, str, strlen(str) + 1, 0) < 0) {
		fprintf(stderr, "Error sending data: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

    // use popen -> fork and connect stdout of child process to stream
    FILE *tarStream = popen(tar_command, "r");
    if (tarStream == NULL) {
        fprintf(stderr, "Error executing tar: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
	
	//ho bisogno di mandare lunghezza file
	//copio archivio in file temporaneo e ne valuto la lunghezza
	FILE *temp_file = tmpfile();
	if (temp_file == NULL) {
        fprintf(stderr, "Error creating temporary file: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

	char buff[BUFFSIZE];
    size_t bytes_read=1;
    while (bytes_read > 0) {
		if(bytes_read = fread(buffer, 1, sizeof(buffer), tarStream)<0){
			fprintf(stderr, "Error reading child process: %s\n", strerror(errno));
            exit(EXIT_FAILURE);			
		}
        if(fwrite(buffer, 1, bytes_read, temp_file)<0){
			fprintf(stderr, "Error wrinting temp file: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
		}
    }


    
	//trovo filesize
	fseek(temp_file, 0, SEEK_END);
    long fileSize = ftell(tempFile);
    fseek(temp_file, 0, SEEK_SET);

	//comunico filesize
	if (send(conn_sd, hton(fileSize), sizeof(fileSize), 0); < 0) {
        fprintf(stderr, "Error sending compressed data: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    //legge stream temp_file e invia bytes a client
    while ((bytes_read = fread(buff, 1, sizeof(buff), temp_file)) > 0) {
        ssize_t snd_bytes = send(conn_sd, buff, bytes_read, 0);
        if (snd_bytes < 0) {
            fprintf(stderr, "Error sending compressed data: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    // Close
    if (fclose(temp_file) < 0) {
        fprintf(stderr, "Error closing temporary file: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (pclose(tarStream) < 0) {
        fprintf(stderr, "Error closing tar process: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
	
}








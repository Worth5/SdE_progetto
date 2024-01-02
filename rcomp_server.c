// questa è la copia dell esempio prof pari pari se compilata ed eseguita permette di connetterci il client per testarlo
#include <arpa/inet.h>   //for inet_pton() INADDR_ANY credo...
#include <dirent.h>      //for struct dirent()
#include <errno.h>       //for errno
#include <fcntl.h>       //for flags like O_RDONLY and SEEK_END
#include <stdio.h>       //for standard I/O
#include <stdlib.h>      //for exit() and EXIT_FAILURE
#include <string.h>      //for strerror()
#include <sys/socket.h>  //for socket
#include <sys/stat.h>    //for stat()
#include <unistd.h>      //for unbuffered functions-> read(), wriqte(), STDIN_FILENO
#include <sys/wait.h>	//for wait()

#define BUFFSIZE 4096
#define PATH_MAX 4096

int sd;//globale per poterlo chiudere da signal handler

char tempFolder[30];//parent 
char recvFolder[] = "rcvdFiles";//inside parent, contains received files 
char compressedFile[] = "compress.temp";//inside parent, compressed output file
//avvio server crea cartella temporanea "parent"
//alla ricezione file li mette nella sottocartella "rcvdFiles"
//alla compressione genera file compresso di nome "compress.temp"
//alla chiusura con quit() elimino la cartella parent
//per evitare che la cartella rimanga usiamo un exit handler

// funzioni main
int setup(int argc, char *argv[]);
int connect_client(int sd);

// funzioni server richieste
void add(int conn_sd);
void compress(int conn_sd, char *compress_type);

// funzioni subordinate
int parse_argv_for_port(int argc, char *argv[]);
int get_size(char *path);

int fread_from_fwrite_to(FILE *s_input, FILE *s_output, int size_to_write, char *progress_bar_message);


//funzioni opzionali
void progress_bar(int processed, int total, char *message);
int extimate_archive_size(int dir_size);


//signal handler
void quit(int signo) {
	close(sd);//close 
	chdir("..");
	int len=sizeof(tempFolder) + sizeof("rm -r ") -1;
	char remove[len];
    snprintf(remove, len, "rm -r %s", tempFolder);
    system(remove);
	exit(EXIT_SUCCESS);
} 


int main(int argc, char *argv[]) {

	snprintf(tempFolder,30, "rcomp_server_temp_%d", getpid());
	//create temp fold
	mode_t mode = S_IRUSR | S_IWUSR | S_IXUSR;
	if (mkdir(tempFolder, mode) < 0) {
		fprintf(stderr, "Error creating directory: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	chdir(tempFolder);

	if(signal(SIGINT, quit) == SIG_ERR){			//Registrazione handler per il segnale SIGINT
		fprintf(stderr, "ERROR: Handler SIGINT registration failed (%s)\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	sd = setup(argc, argv);  // crea socket imposta sockopt esegue bind e listen

    while (1) {
		int conn_sd = connect_client(sd);  // esegue connet e restituisce connection socket
		int valid_input = 1;
		char command[10] = "0";

        while (valid_input && (strcmp(command, "q")!=0)) {  // finche input è valido e diverso da "q"

            if (recv(conn_sd, &command, 2, 0) < 0) {
                fprintf(stderr, "Impossibile ricevere dati da socket: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }

            if (strcmp(command, "a")==0) {
                add(conn_sd);  // riceve file e lo salva
            }
            else if ((strcmp(command, "j")==0) || (strcmp(command, "z")==0)) {
                compress(conn_sd, command);  // fork al comando tar con exec poi manda il file al client
            }
            else if (strcmp(command, "q")==0) {
                printf("Connection interrupted by client\n");
            }
            else {
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

///////////////////////////////////////////////////

int setup(int argc, char *argv[]) {  // create socket + socket opt + bind() return socket

    int port_no = 1234;
    int port_pos = parse_argv_for_port(argc, argv);
    if (port_pos > 0) {
        port_no = atoi(argv[port_pos]);
        printf("port:%d\n", port_no);
    }
    else {
        printf("no valid port, using default:%d\n", port_no);
    }

    int sd;
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "Impossibile creare il socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    char *addr_str = "127.0.0.1";
    in_addr_t address;
    if (inet_pton(AF_INET, addr_str, (void *)&address) < 0) {  // conversione dell'indirizzo in formato numerico
        fprintf(stderr, "Impossibile convertire l'indirizzo: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in sa;  // preparazione della struttura contenente indirizzo IP e porta
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port_no);
    sa.sin_addr.s_addr = address;
    int reuse_opt = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &reuse_opt, sizeof(int));

    // --- BIND --- //
    if (bind(sd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {  // associazione indirizzo a socket
        fprintf(stderr, "Impossibile associare l'indirizzo a un socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("Socket %d associato a %s:%d\n", sd, addr_str, port_no);

    // --- LISTENING --- //
    if (listen(sd, 10) < 0) {
        fprintf(stderr, "Impossibile mettersi in attesa su socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return sd;
}

/////////////////////////////////////////////////////////////

int parse_argv_for_port(int argc, char *argv[]) {
    // debug("parse_argv_for_port()\n",3);
    for (int j = 1; j < argc; j++) {
        // debug("arg\n",4);
        int check = 1;
        for (int i = 0; argv[j][i] != '\0'; i++) {
            char c[3] = {i + '0', '\n', '\0'};
            // debug(c,5);
            if (argv[j][i] < '0' || argv[j][i] > '9') {
                check = 0;
            }
        }
        if (check) {
            // debug("valid port found\n",3);
            return j;
        }
    }
    // debug("no arg has valid port\n",3);
    return -1;
}

//////////////////////////////////////////////////////////////

int connect_client(int sd) {
    // --- ATTESA DI CONNESSIONE --- //
    printf("--- In attesa di connessione ---\n");

    int conn_sd;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    if ((conn_sd = accept(sd, (struct sockaddr *)&client_addr, &client_addr_len)) < 0) {
        fprintf(stderr, "Impossibile accettare connessione su socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // conversione dell'indirizzo in formato numerico
    char client_addr_str[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_addr_str, INET_ADDRSTRLEN) < 0) {
        fprintf(stderr, "Impossibile convertire l'indirizzo\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("Connesso al client %s:%d\n", client_addr_str, ntohs(client_addr.sin_port));
    return conn_sd;
}

//////////////////////////////////////////////////////

void add(int conn_sd) {
    printf("Add request received\n");

    // mkdir se dir non esiste
    mode_t mode = S_IRUSR | S_IWUSR | S_IXUSR;
	if (mkdir(recvFolder, mode) < 0) {
		if(errno != EEXIST){
			fprintf(stderr, "Error creating directory: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
    
    // sposto working directory in recvFolder
    chdir(recvFolder);

    // ricevi lunghezza nome del file
    int file_name_len;
    if (recv(conn_sd, &file_name_len, sizeof(int), 0) < 0) {
        fprintf(stderr, "Error receiving size of file name: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    file_name_len = ntohl(file_name_len);

    // alloco spazio in memoria e ricevo stringa nome del file
    char filename[file_name_len];
    if (recv(conn_sd, &filename, file_name_len, 0) < 0) {
        fprintf(stderr, "Error receiving file name: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // check file gìà presente in questo caso non si fa niente e verrà sovrascritto
    struct stat file_info;
    if (stat(filename, &file_info) == 0) {
        printf("File alreaddy exists, overwriting with new file\n");
    }

    // crea file all'interno della dir con nome ricevuto
    int fd;
    if ((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 644)) < 0) {//   xwr xwr xwr -> 111 111 111-> 7 7 7 ->  644 = xwr --r --r
        fprintf(stderr, "ERROR: cannot create file %s (%s)\n", filename, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // ricevi grandezza file
    int file_size;
    int rcvd_bytes = recv(conn_sd, &file_size, sizeof(file_size), 0);
    if (rcvd_bytes < 0) {
        fprintf(stderr, "Error receiving file size: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    file_size = ntohl(file_size);

    // ciclo recv from socket, write on file
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
		progress_bar(file_size-remaining_bytes,file_size,"Receiving file");
    }

    printf("\nFile received and saved as: %s\n", filename);

    // Close the file descriptor

    close(fd);
    chdir("..");  // gestione errore?
}

//////////////////////////////////////////////////////////////////

void compress(int conn_sd, char *comp_type) {
    printf("Compress request received2\n");

    // children commands
    char tarCommand[50];
	char compressCommand[50];

	//creo comandi per processi figli
	snprintf(tarCommand,50,"tar cf - -C %s .",recvFolder);
	if(strcmp(comp_type,"j")){
		snprintf(compressCommand,50,"gzip > %s",compressedFile);
	}
	else{
		snprintf(compressCommand,50,"bzip2 > %s", compressedFile);
	}

	//controllo se ci sono file da comprimere
	//se non ce ne sono ritorno
	int dirSize = get_size(recvFolder);
	if(dirSize<0){
		int snd_bytes = send(conn_sd, "NO", 3, 0);// {'N','O','\0'}
		if (snd_bytes < 0) {
			fprintf(stderr, "Error sending compressed data: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		return;
	}
	else{
		int snd_bytes = send(conn_sd, "OK", 3, 0);
		if (snd_bytes < 0) {
			fprintf(stderr, "Error sending compressed data: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
    printf("UncompressedSize: %ld\n", dirSize);
	int archiveExtimate=extimate_archive_size(dirSize);//get extimate of archive size to use for progress bar

	//ho separato il comando in due per poter printare una progress bar
	FILE *tarStream= popen(tarCommand,"r");
	FILE *compressStream= popen(compressCommand,"w");
	fread_from_fwrite_to(tarStream,compressStream,archiveExtimate,"Compression");//qui passo dati da un figlio all'altro e printo progress bar
	fclose(tarStream);
	fclose(compressStream);

	//faccio una wait per aspettare che i processi figli abbiano terminato
	while(wait(NULL) > 0);
	if(errno!=ECHILD){//errore no figli 
		fprintf(stderr,"Error on Wait: %s",strerror(errno));
		exit(EXIT_FAILURE);
	}


	int compressedSize = get_size(compressedFile);
	printf("Compressed size: %ld\n", compressedSize);
	// mando lunghezza file
	compressedSize=htonl(compressedSize);
	if (send(conn_sd, &compressedSize, sizeof(compressedSize), 0) < 0) {
		fprintf(stderr, "Error sending data: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	FILE *zip = fopen(compressedFile,"r");
	// mando file
	char buff[BUFFSIZE];
	int bytes_read, total_sent=0;
	while ((bytes_read = fread(buff, 1, sizeof(buff), zip)) > 0) {
		ssize_t snd_bytes = send(conn_sd, buff, bytes_read, 0);
		if (snd_bytes < 0) {
			fprintf(stderr, "Error sending compressed data: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		total_sent+=snd_bytes;
		progress_bar(total_sent,compressedSize,"Sending");
	}
	fclose(zip);

	//rimuovo cartella con file ricevuti
	printf("Removing uncompressed files\n");
	int len=sizeof(recvFolder) + sizeof("rm -r ") -1;
	char remove[len];
    snprintf(remove, len, "rm -r %s", recvFolder);
    system(remove);

}

/////////////////////////////////////////////////////////////////////////////

int get_size(char *path) {
    // controllo che file esista
    struct stat fileStat;
    if (stat(path, &fileStat) == -1) {
        // file non trovato uso questo ritorno per capire se ci sono file da comprimere
        if (errno == ENOENT) {  // ENOENT
            return -1;
        }
        else {  // errore diverso in apertura
            fprintf(stderr, "ERROR: cannot open path '%s': (%s)\n", path, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

	if(S_ISREG(fileStat.st_mode)){ //se file è file regolare ritorno dimensione
		return fileStat.st_size;
	}

	else if(S_ISDIR(fileStat.st_mode)){//se file è directory scorri elementi
		DIR *dp = opendir(path);
		if (dp == NULL) {
			printf("Can't open directory '%s': %s", path, strerror(errno));
			exit(EXIT_FAILURE);
		}

		//cartella
        	//dirent 1
        	//dirent 2
        	//dirent n
		struct dirent *dirp;  // dirent = dir entries
		struct stat file_stat;
		int total_size = 0;

		// readdir()  -> dirent numero 1 ->[puntatore a i-node, nomefile]
    	// readdir() -> dirent nomero 2 ->
		while ((dirp = readdir(dp)) != NULL) {

			if ((strcmp(dirp->d_name, ".") == 0) || (strcmp(dirp->d_name, "..") == 0)){//dir ignora se stessa e genitore
                continue;//va a prox ciclo
			}
			int size;
			char file_path[PATH_MAX];
			snprintf(file_path, sizeof(file_path), "%s/%s", path, dirp->d_name);

			if((size = get_size(file_path)) >= 0){//recursion
				total_size += size;
			}
		}
    closedir(dp);
    return total_size;
	}
}

////////////////////////////////////////////////////

int fread_from_fwrite_to(FILE *s_input, FILE *s_output, int size_to_write, char *progress_bar_message) {

    char buffer[BUFFSIZE];
    int bytes_read;
    int total_read = 0;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), s_input)) > 0) {
        // printf("bytes_read: %d \n", bytes_read);

        int bytes_written = fwrite(buffer, 1, bytes_read, s_output);

        // printf("bytes_written: %d \n", bytes_written);
        total_read += bytes_written;
        // printf("total read: %ld ", total_read);

        if (bytes_written < bytes_read) {
            fprintf(stderr, "ERROR accessing file: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (size_to_write) {
            progress_bar(total_read, size_to_write, progress_bar_message);
        }
    }
    fflush(s_output);
    if (ferror(s_input)) {
        fprintf(stderr, "ERROR reading stream: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (size_to_write) {
        printf("\n");  // per avere \n dopo progress bar
    }
    // return filesize;
    return total_read;
}

/////////////////////////////////////////////////////////////////////////

void progress_bar(int processed, int total, char *message) {
    printf("Processed= %ld    ", processed);
    int progress = (int)((processed / (float)total) * 100);
    printf("%s progress: [%d%%]\r", message, progress);//  %d per intero  %% per scrivere "%"  -> "message progress:[n%]
    fflush(stdout);
}

///////////////////////////////////////////////7

int extimate_archive_size(int dir_size) {  // ricavata a tentativi per ora corretta
    int expected_size = dir_size - (dir_size % 10240) + 10240;
    if ((dir_size % 10240) <= 8192)
        return expected_size;
    else
        return expected_size + 10240;
}



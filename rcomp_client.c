#include <stdint.h>				//for uint8_t type
#include <stdio.h>				//for standard I/O
#include <string.h>				//for strerror()
#include <errno.h>				//for errno
#include <stdlib.h>				//for exit() and EXIT_FAILURE
#include <unistd.h>				//for unbuffered functions-> read(), write(), STDIN_FILENO
#include <signal.h>				//for signal handler
#include <fcntl.h>				//for open()
#include <sys/socket.h>			//for int socket(int domain, int type, int protocol);
#include <arpa/inet.h>			//for uint16_t htons(uint16_t data);
#include <sys/stat.h>			//for stat() file size

//abilita printf di debug all'interno dell funzione debug()
#define DEBUG_LEVEL 0 //0  disabilita tutte printf di debug, più è alto il valore maggiori saranno le printf...


//creo una struttura che contiene comando e argomento
struct request{					
	char *command;			
	char *argument;

};

//funzioni nel main
int setup (int argc, char* argv[]);
void get_request(struct request rq);                    
void manage_request(int sd, struct request rq);

//funzioni subordinate
int fget_word(FILE* fd, char* str);
int parse_argv_for_ip(int argc, char* argv[]);
int parse_argv_for_port(int argc, char* argv[]);

//funzioni accessorie/opzionali
void debug(char *str, int level){	//stampa in verde messaggio debug
	#ifdef DEBUG_LEVEL
		if (DEBUG_LEVEL >= level){
			while(level--)printf("\t");	//indenta level volte per visibilità
			printf("\033[0;34m%s\033[1;0m", str); //colore verde \033[1;32m colore normale \033[1;0m
		}
	#endif
}

//funzioni client richieste
void help();
void add(int sd, char* argument);
void compress(int sd, char* argument);
void quit(int sd, struct request rq);

//signal handler
void sigint_handler(int signo) {
	int fd[2];
	pipe(fd);
	close(STDIN_FILENO);
	dup2(fd[0],STDIN_FILENO);

	dprintf(fd[1],"quit\n");
	debug("sigint\n",1);

} 

//--------------- MAIN --------------- //
int main(int argc, char *argv[]){
	debug("main()_start\n",1);

	if(signal(SIGINT, sigint_handler) == SIG_ERR){			//Registrazione handler per il segnale SIGINT
		fprintf(stderr, "ERROR: Handler SIGINT registration failed (%s)\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	int sd = setup(argc, argv); // gestisce la creazione socket e connessione al server
	struct request rq = {.command = malloc(1), .argument = malloc(1)};//crea struttura request che viene usata ogni ciclo e eliminata in quit
	
	do{
		debug("main()_while\n",2);
		get_request(rq); 		// alloco memoria necessaria a contenere stringa richiesta si potrebbe fare più efficente salvando nella struttura 2 variabili LEN1 e LEN2 che salvano lo spazio già allocato per comando e argomnto
		manage_request(sd, rq);	// controllo contenuto request e chiamo la funzione principale corrispondente
	}while (strcmp(rq.command, "quit") != 0);

	debug("main()_quit\n",1);
	quit(sd, rq);				//manda mess a server chiude socket libera memoria allocata e esegue exit(EXIT_SUCCESS)
}



int setup(int argc, char* argv[]){
	debug("setup()\n",2);
    
    char addr_str[16] = "127.0.0.1";
	int ip_pos = parse_argv_for_ip(argc, argv);
	if(ip_pos > 0){
		strcpy(addr_str,argv[ip_pos]);
		printf("ip address:%s\n",addr_str);
	}
	else {
		printf("no valid ip address, using default:%s\n",addr_str);
	}

	int port_no = 1234;
	int port_pos = parse_argv_for_port(argc, argv);
	if(port_pos > 0){
		port_no = atoi(argv[port_pos]);
		printf("port:%d\n",port_no);
	}
	else{
		printf("no valid port, using default:%d\n",port_no);
	}
    
	//trasfmormo ip in formato numerico richiesto
	in_addr_t address;
    if (inet_pton(AF_INET, addr_str, (void*)&address) < 0){ 
        fprintf(stderr, "Failed to convert address: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

	//creo socket
    int sd;
    if ( (sd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        fprintf(stderr, "Failed to create socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } 

	// preparazione della struttura contenente indirizzo IP e porta
    struct sockaddr_in sa;				
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port_no);
    sa.sin_addr.s_addr = address;

	//connessione
	printf("Connecting to server %s:%d ...",addr_str, ntohs(sa.sin_port));
	fflush(stdout);
	if (connect(sd, (struct sockaddr*)&sa, sizeof(struct sockaddr_in)) < 0){
		fprintf(stderr, "\nConnection failed: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	printf("success!\n");
	return sd;							//ritorno socket descriptor
}    

int parse_argv_for_ip(int argc, char* argv[]){
	debug("parse_argv_for_ip()\n",3);
	int byte[4];
	for(int j = 1; j < argc; j++){

		if(sscanf(argv[j],"%d.%d.%d.%d",&byte[0],&byte[1],&byte[2],&byte[3])==4){//controllo quale argomento ha formato ip
			debug("parse_argv_for_ip() checking possible ip\n",4);
			int check = 1;
			for(int i = 0; i< (sizeof(byte) / sizeof(byte[0])); i++){
				
				if((byte[i]<0) || (byte[i]>255)){	//controllo che sia ip valido
					check = 0;
				}
			}
			if(check){
				debug("valid ip found\n",3);
				return j; // ritorno numero argomento argc valido
			}
		}
	}
	debug("no arg has valid ip\n",3);
	return -1;
}

int parse_argv_for_port(int argc, char* argv[]){

	debug("parse_argv_for_port()\n",3);
	for(int j = 1; j < argc; j++){
		debug("arg\n",4);
		int check=1;
		for(int i = 0; argv[j][i]!='\0'; i++){
			char c[3]={i+'0','\n','\0'};
			debug(c,5);
			if(argv[j][i]<'0' || argv[j][i]>'9'){
				check=0;
			}
		}
		if(check){
			debug("valid port found\n",3);
			return j;
		}
		
	}
	debug("no arg has valid port\n",3);
	return -1;
}

//vecchio get request
/*
struct request get_request_vecchio(){	

	printf("rcomp> ");
	struct request rq = {0};	//struttura inizializzata a zero
	char input[120]; 
	
    if (fgets(input, sizeof(input), stdin) == NULL) {
        fprintf(stderr, "Error reading input: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (sscanf(input, "%9s %99s[^\n]", rq.command, rq.argument) < 1) {	//[^\n] significa che non accetta \n come carattere quindi non assegna rq.argument se c'è solo rd.command si trova in man sscanf
		printf("No input detected\n");
    }
	
	return rq;
}
*/


void get_request(struct request rq){ //il nuovo get request assegna memoria dinamicamente

	printf("rcomp> ");
	//modo carino di leggere tutto stdin dinamicamente ma che non serve nel nostro caso
	/*
	int STEP = 0;
	int SIZE = 10;
	char *input = (char *)malloc(SIZE);
	while (fgets(input + (STEP * SIZE), SIZE, stdin) != NULL){
		input = (char *)realloc(input, SIZE * (++STEP));
	}
	if (ferror){
		fprintf(stderr, "Error reading input: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	free(input);
	*/
	debug("\n",3);
	debug("get_request()_reset_struct\n",3);
	rq.argument[0] = '\0'; // resetta struttura request che contiene ancora vecchi valori;
	rq.command[0] = '\0';

	//prendo due parole da stdin poi svuoto stdin se fosse rimasto qualcosa
	debug("get_request()_get_word_1\n",3);	
	int temp = fget_word(stdin, rq.command); 	//prima parola

	if((temp != '\n') && (temp != '\0') && (temp != EOF)){
		debug("get_request()_get_word_2\n",3);
		temp = fget_word(stdin, rq.argument);	//seconda parola
	}

	if((temp != '\n') && (temp != '\0') && (temp != EOF)){
		debug("get_request()_flush_exess_stdin\n",4);
		while (((temp = getchar()) != '\n') && (temp != EOF))	//svuoto stdin
			debug((char*)&temp,5);
	}

	printf("Command: %s\n", rq.command);
	printf("Argument: %s\n", rq.argument);
	
}

int fget_word(FILE* fd, char* str){
	int byte;
	int SIZE = 2;

	debug("fget_word()_remove_spaces\n",4);
	while(((byte = fgetc(fd)) == ' ') || (byte == '\n')){	//remove leading white spaces
		
	}
	
	if (byte < 0){
			printf("ERROR while reading input: %s",strerror(errno));
			//exit(EXIT_FAILURE);
		}
	debug("fget_word()_ungetc()\n",4);
	ungetc(byte, fd);		//rimette nello stream il primo non_white character
	
	debug("fget_word()_get_letters\n",4);
	while ( ((byte = fgetc(fd)) != '\n') && (byte != ' ') && (byte != EOF) ){
		debug((char*)&byte,5);
		debug("\n",5);
		if (byte < 0){
			printf("ERROR while reading input: %s",strerror(errno));
			exit(EXIT_FAILURE);
		}
		
		str = (char *)realloc(str, SIZE * sizeof(char)); 
		SIZE++;
		strncat(str, (char *)&byte, 1);
	}
	return byte;
}

void manage_request(int sd, struct request rq){
	debug("manage_request()\n",3);	
	if(strcmp(rq.command,"help") == 0){
		help();
	}	
	else if(strcmp(rq.command,"add") == 0){
		add(sd, rq.argument);
	}
	else if(strcmp(rq.command,"compress") == 0){
		compress(sd, rq.argument);
	}
	else if(strcmp(rq.command,"quit") != 0){
		printf("ERROR: unknown command\n");
	}
}

void help(){
	debug("help()",4);
	const char* string =
	    "Comandi disponibili\n"
	    "\thelp:\n"
	    "\t--> mostra l’elenco dei comandi disponibili add [file]\n"
		"\tadd:\n"
	    "\t--> invia il file specificato al server remoto compress [alg]\n"
		"\tcompress:\n"
	    "\t--> riceve dal server remoto l’archivio compresso secondo l’algoritmo specificato\n"
	    "\tquit\n"
	    "\t--> disconnessione\n";
	printf(string);
}

void add(int sd, char* argument){
	debug("add()_argument:",4);					//debug se printa sei nella funzione
	debug(argument,4);
	
	for(int i = 0; argument[i] != '\0'; i++){
		if((argument[i] < 'A' || argument[i] > 'Z') && (argument[i] < 'a' || argument[i] > 'z') && (argument[i] < '0' || argument[i] > '9') && argument[i] != '.'){
			fprintf(stderr, "ERROR: Invalid file name\n");
			return;
		}
	}


	int fd;

	if((fd = open(argument, O_RDONLY)) < 0){
		fprintf(stderr, "ERROR: File %s not found (%s)\n", argument, strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	struct stat metadata;
	if(stat(argument, &metadata) < 0){
		fprintf(stderr, "ERROR: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	off_t file_size = metadata.st_size;

	ssize_t snd_bytes;

	//------------INVIO COMANDO AL SERVER---------------------//
	cahr *str = "a";
	if((snd_bytes = send(sd, str, strlen(str)+1, 0)) < 0){
		fprintf(stderr, "ERROR: Impossible to send data on socket (%s)\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	//------------INVIO LUNGHEZZA "FILE NAME"-----------------//
	size_t file_name = htonl(strlen(argument)+1);
	if((snd_bytes = send(sd, &file_name, sizeof(int), 0)) < 0){
		fprintf(stderr, "ERROR: Impossible to send data on socket (%s)\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	//-----------------INVIO "FILE NAME"----------------------//
	if((snd_bytes = send(sd, argument, strlen(argument)+1, 0)) < 0){
		fprintf(stderr, "ERROR: Impossible to send data on socket (%s)\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	//------------INVIO LUNGHEZZA----------------//

	int file_size_n = htonl(file_size);
	if((snd_bytes = send(sd, &file_size_n, sizeof(int), 0)) < 0){
		fprintf(stderr, "ERROR: Impossible to send size file (%s)\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	//------------INVIO MESSAGGIO----------------//

	const int BUFFSIZE = 4096;
	char buff[BUFFSIZE];
	int n;

	while((n = read(fd, buff, BUFFSIZE)) > 0){
		if((snd_bytes = send(sd, buff, n, 0)) < 0){
			fprintf(stderr, "ERROR: Impossible to send message (%s)\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	if(n < 0){
		fprintf(stderr, "ERROR while reading\n");
		exit(EXIT_FAILURE);
	}	
	
	printf("File %s sent\n", argument);
}

void compress(int sd, char* argument){
	debug("compress()_argument:\n",4);				//debug se printa sei nella funzione
	debug(argument,4);
	ssize_t rcvd_bytes, snd_bytes;

	if (strcmp(argument, "z") == 0 || strcmp(argument, "j") == 0){ 
           if ((snd_bytes = send(sd, argument, strlen(argument), 0)) < 0){ // Dico al server quale algoritmo usare
			fprintf(stderr, "Error : Unable to specify the algorithm: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
	   }
		printf("Messaggio inviato al server: %s\n", argument);
	}else if (strcmp(argument, "") == 0){
		strcpy(argument, "z");
		if ((snd_bytes = send(sd, argument, strlen(argument) + 1, 0)) < 0) {
			fprintf(stderr, "Error : Unable to specify the algorithm: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		printf("Message sent to the server: %s (Empty argument, set to 'z')\n", argument);
	}else{
		printf("Error: Invalid argument, use 'z' for gzip or 'j' for bzip2.\n");
	}

        // --- RICEZIONE RISPOSTA --- //
        char resp[3];
        rcvd_bytes = recv(sd, &resp, 2, 0);
        if (rcvd_bytes < 0)
        {
            fprintf(stderr, "ERRORE: aggiungere almeno un file prima di effettuare la compressione: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        resp[rcvd_bytes] = '\0';
        char *filename[30];
        if (strcmp(resp, "OK") == 0) {
           if (strcmp(argument, "z") == 0){
              strcpy(filname,"archiviocompresso.tar.gz");
	   }else{
	      strcpy(filneme,"archiviocompresso.tar.bz2");
	   }
		
		FILE *myfile = fopen(filename, "wb");
	   	if (myfile == NULL) {
	      fprintf(stderr, "Errore: Impossibile aprire il file per la ricezione\n");
              exit(EXIT_FAILURE);
	   }

		//ricevere grandezza file
		//long file_size=
			
	   FILE *socket_stream =fdopen(socked_descriptor, "r");
	   if ((soclet_stream = fopen(filename_in, "r")) == NULL){
              fprintf(stderr,"ERROR while opening %s\n", filename_in); 
              exit(EXIT_FAILURE);
	   }
		
            char buffer[BUFFSIZE];
	      size_t bytes_read;
	      long total_read = 0;
              while ( total_read<file_size) {
				rcvd_bytes = recv(conn_sd, &buffer, sizeof(buffer), 0);
		   		size_t bytes_written = fwrite(buffer, 1, recvd_bytes, myfile);
		   		total_read += bytes_written;
		   if (bytes_written < recvd_bytes) {
		      fprintf(stderr, "Error writing to temporary file: %s\n", strerror(errno));
		      exit(EXIT_FAILURE);
		   }
	    } 
		//chiudere il file con controllo errore
		//printf 
	}
}

void quit(int sd, struct request rq) {
	debug("quit()\n",4);					//debug se printa sei nella funzione
	char *str = "q";
	int snd_bytes;

	if ((snd_bytes = send(sd, &str, strlen(str)+1, 0)) < 0){	//invio quit al server
		fprintf(stderr, "Impossibile inviare dati su socket: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	free(rq.command);
	free(rq.argument);
	close(sd);
	exit(EXIT_SUCCESS);
}


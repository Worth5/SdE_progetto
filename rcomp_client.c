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


struct request{					//idea creo una struttura che contiene comando e argomento
	char *command;			
	char *argument;
};

void sigint_handler(int signo){quit()};
int setup (int argc, char* argv[]);
struct request get_request();                    
void manage_request(int sd, struct request rq);
uint8_t check_valid(char* command);
void help();
void add(int sd, char* argument);
void compress(int sd, char* argument);
void quit();

int main(int argc, char *argv[])
{
	if(signal(SIGINT, sigint_handler) == SIG_ERR){													//Registrazione handler per il segnale SIGINT
		fprintf(stderr, "ERROR: Handler SIGINT registration failed (%s)\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	int sd = setup(argc, argv); // gestisce la creazione socket e connessione al server
	struct request rq = {0};

	while (strcmp(rq.command, "quit") != 0){
		rq = get_request(); // creo struttura request
		manage_request(sd, rq);
	}
	quit(sd);//manda mess a server chiude socket e esegue exit(EXIT_SUCCESS)
}

int setup(int argc, char* argv[]){

    char* addr_str = "127.0.0.1";
    int port_no = 1234;
    in_addr_t address;

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
	
    if (inet_pton(AF_INET, addr_str, (void*)&address) < 0){
        fprintf(stderr, "Failed to convert address: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }


    int sd;
    if ( (sd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        fprintf(stderr, "Failed to create socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } 

	
    struct sockaddr_in sa;				// preparazione della struttura contenente indirizzo IP e porta
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port_no);
    sa.sin_addr.s_addr = address;

	
	printf("Connecting to server %s:%d ...",addr_str, ntohs(sa.sin_port));
	fflush(stdout);
	if (connect(sd, (struct sockaddr*)&sa, sizeof(struct sockaddr_in)) < 0){
		fprintf(stderr, "\nConnection failed: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	printf("success!\n");
	return sd;							//ritorno socket descriptor
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

struct request get_request(){ //il nuovo get request assegna memoria dinamicamente

	printf("rcomp> ");
	struct request rq = {.command=NULL, .argument=NULL}; // struttura inizializzata a zero

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
	int c;
	int STEP = 2;
	while ((c = getchar()) != '\n'){
		if (c < 0){
			printf("ERROR while reading input: %s",strerror(errno));
			exit(EXIT_FAILURE):
		}
		rq.command = (char *)realloc(rq.command, STEP++);
		if (c == " "){
			break;
		}
		strcat(rq.command, &(char)c);
	}
	if (rq.command == NULL){
		printf("no command detected\n");
	}
	else{
		printf("command: %s\n", rq.command);
	}

	STEP = 2;
	if (c != '\n'){
		while ((c = getchar()) != '\n' && c != EOF){
			if (c < 0){
				printf("ERROR while reading input: %s",strerror(errno));
				exit(EXIT_FAILURE):
			}
			rq.command = (char *)realloc(rq.argument, STEP++);
			if (c == " "){
				break;
			}
		strcat(rq.command, &c);
	}
	if (rq.argument == NULL){
		printf("No argument detected\n");
	}
	else{
		printf("argument: %s\n", rq.argument);
	}

	return rq;
}


void manage_request(int sd, struct request rq){

	if(!strcmp(rq.command,"help")){
		help();
	}	
	else if(!strcmp(rq.command,"add")){
		add(sd, rq.argument);
	}
	else if(!strcmp(rq.command,"compress")){
		compress(sd, rq.argument);
	}
	else if(strcmp(rq.command,"quit")){
		printf("ERROR: unknown command\n";)
	}
}

void help(){
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
	printf("add command\n");					//debug se printa sei nella funzione
	printf("file path: '%s'\n",argument);
	
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

	//------------INVIO LUNGHEZZA----------------//

	ssize_t snd_bytes;
	int file_size_n = htonl(file_size);
	if((snd_bytes = send(sd, &file_size_n, sizeof(int), 0)) < 0){
		fprintf(stderr, "ERROR: Impossible to send size file (%s)\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	//------------INVIO MESSAGGIO----------------//

	const int BUFFSIZE = 4096;
	char buff[BUFFSIZE];
	int n;

	printf("Sending file %s to the current server...\n", argument);
	
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

	printf("compress command\n");				//debug se printa sei nella funzione
	printf("compress method: '%s'\n",argument);
	ssize_t snd_bytes;


	if (strcmp(argument, "z") == 0 || strcmp(argument, "j") == 0){
		if ((snd_bytes = send(sd, argument, strlen(argument), 0)) < 0){ // Dico al server quale algoritmo usare
			printf(stderr, "Impossibile inviare dati: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		printf("Messaggio inviato al server: %s\n", argument);
	}
	else{
		printf("Errore: Algoritmo non valido. Utilizzare 'z' per gzip o 'j' per bzip2.\n");
	}
}

void quit(int sd) {
	printf("quit command\n");					//debug se printa sei nella funzione
	char *c = "q";

	if ((snd_bytes = send(conn_sd, &c, sizeof(c), 0)) < 0){
		fprintf(stderr, "Impossibile inviare dati su socket: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	close(sd);
	exit(EXIT_SUCCESS);
}


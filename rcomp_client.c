#include <stdint.h>				//for uint8_t type
#include <stdio.h>				//for standard I/O
#include <string.h>				//for strerror()
#include <errno.h>				//for errno
#include <stdlib.h>				//for exit() and EXIT_FAILURE
#include <unistd.h>				//for unbuffered functions-> read(), write(), STDIN_FILENO
#include <sys/socket.h>			//for int socket(int domain, int type, int protocol);
#include <arpa/inet.h>			//for uint16_t htons(uint16_t data);


struct request{					//idea creo una struttura che contiene comando e argomento
	char command[10];			
	char argument[100];		
	uint8_t valid;
};

int setup (int argc, char* argv[]);
struct request get_request();                    
void manage_request(int sd, struct request rq);
uint8_t check_valid(char* command);
void help();
void add(int sd, char* argument);
void compress(int sd, char* argument);
void quit();


int main(int argc, char* argv[]){
	int sd=setup(argc, argv);   	//gestisce la creazione socket e connessione al server
	struct request rq;
    do{
        rq = get_request(); 	//creo struttura request
        manage_request(sd, rq);
    } while (strcmp(rq.command, "quit") != 0);
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

struct request get_request(){
	printf("rcomp> ");
	struct request rq;
	char input[120]; 
	
    if (fgets(input, sizeof(input), stdin) == NULL) {
        fprintf(stderr, "Error reading input: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (sscanf(input, "%9s %99s[^\n]", rq.command, rq.argument) < 1) {	//[^\n] significa che non accetta \n come carattere quindi non assegna rq.argument se c'è solo rd.command si trova in man sscanf 
        rq.command[0]='\0';
		rq.argument[0]='\0';
		printf("No input detected\n");
    }
	else if((rq.valid = check_valid(rq.command)) == 0){
		printf("Command not recognised\n");
	}
	
	return rq;
}

uint8_t check_valid(char* cmd){
	if(strcmp(cmd,"help") && strcmp(cmd,"add") && strcmp(cmd,"compress") && strcmp(cmd,"quit"))
		return 0;
	else 
		return 1;
}


void manage_request(int sd, struct request rq){
	if(rq.valid){
		if(!strcmp(rq.command,"help"))
			help();
			
		else if(!strcmp(rq.command,"add"))
			add(sd, rq.argument);
			
		else if(!strcmp(rq.command,"compress"))
			compress(sd, rq.argument);
			
		else if(!strcmp(rq.command,"quit"))
			quit();
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
		if(argument[i] < 'A' || argument[i] > 'Z' && argument[i] < 'a' || argument[i] > 'z' && argument[i] < '0' || argument[i] > '9' && argument[i] != '.'){
			fprintf(stderr, "ERROR: Invalid file name\n");
			return;
		}
	}

	//codice per inviare il file
	
}

void compress(int sd, char* argument){
	printf("compress command\n");				//debug se printa sei nella funzione
	printf("compress method: '%s'\n",argument);
	//codice
}

void quit() {
	printf("quit command\n");					//debug se printa sei nella funzione
    // code for quitting
}


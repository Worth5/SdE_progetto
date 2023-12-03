//alla fine controllare di lasciare 
#include <stdint.h>				//for uint8_t
#include <stdio.h>				//for standard I/O
#include <string.h>				//for strerror()
#include <errno.h>				//for errno
#include <fcntl.h>				//for flags like O_RDONLY and SEEK_END
#include <stdlib.h>				//for exit() and EXIT_FAILURE
#include <unistd.h>				//for unbuffered functions-> read(), write(), STDIN_FILENO
#include <sys/socket.h>			//for int socket(int domain, int type, int protocol);
#include <arpa/inet.h>			//for uint16_t htons(uint16_t data);


struct request{					//idea creo una struttura che contiene comando e argomento
    char command[10];			//deve poter contenere al max "compress" quindo arrotondiamo a 10
    char* argument[100];		//deve poter contenere indirizzo file quinddi potrebbe essere lungo (100 è esagerato)
	bool valid;
};
int setup (int argc, char* argv[]);
request get_request();                    
void manage_request(request* rq);
bool check_valid(char* command);
void help();
void add();
void compress();


int main(int argc, char* argv[]){

	int sd=setup(argc, argv);   	//gestisce la creazione socket e connessione al server
	
    do{
        request rq = get_request(); 	//creo struttura request
        manage_request(sd, req);
    }
    while(strcomp(rq.command,"quit"));
}



int setup(int argc, char* argv[]){

    char* addr_str = "127.0.0.1";
    int port_no = 1234;
    in_addr_t address;

    if(argc == 3){
        if(strchr(argv[1],'.')!=NULL){
            char* addr_str=argv[1];
            port_no = atoi(argv[2]); 	//manca controllo errore.... meglio cmbiare funzione completamente esempio strtol()
        }
        else {
            char* addr_str=argv[2]
            port_no = atoi(argv[1]); 	//manca controllo errore.... meglio cmbiare funzione completamente
        }
	}
    else if(argc == 2){
        if(strchr(argv[1],'.')!=NULL)
			char* addr_str=argv[1];
		else
			port_no=atoi(argv[1]);
	}
	
	in_addr_t address;
    if (inet_pton(AF_INET, addr_str, (void*)&address) < 0){
        fprintf(stderr, "Impossibile convertire l'indirizzo: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }


    int sd;
    if ( (sd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        fprintf(stderr, "Impossibile creare il socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } 

	
    struct sockaddr_in sa;				// preparazione della struttura contenente indirizzo IP e porta
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port_no);
    sa.sin_addr.s_addr = address;

	
printf("Connessione...\n");				//connessione al server
if (connect(sd, (struct sockaddr*)&sa, sizeof(struct sockaddr_in)) < 0){
	fprintf(stderr, "Impossibile connettersi: %s\n", strerror(errno));
	exit(EXIT_FAILURE);
    }
	return sd;							//ritorno socket descriptor
  }    

request get_request(){
	printf("rcomp> ");
	request rq;

	if (scanf("%9s", &rq.command) != 1) {
        // Error reading the first string
        fprintf(stderr, "Error reading input: %s\n",strerror(errno));
        exit(EXIT_FAILURE);
    }
	if (scanf("%99s", &rq.argument) != 1) {
        rq.argument = NULL;
		if(ferror(stdin)){
			fprintf(stderr, "Error reading input: %s\n",strerror(errno));
       		exit(EXIT_FAILURE);
    	}
    }
if((rd.valid = check_valid(rq.command)) == 0)
	printf("Command not recognised\n");

return rd;
}

bool check_valid(char* cmd){
	if(strcmp(cmd,"help") && strcmp(cmd,"add") && strcmp(cmd,"compress") && strcmp(cmd,"quit"))
		return 0;
	else return 1;
}


void manage_request(int sd, request rd){
	if(rq.valid){
		if(!strcmp(rd.command,"help"))
			help();
		else if(!strcmp(rd.command,"add"))
			add(sd, rd.argument);
		else if(!strcmp(rd.command,"compress"))
			compress(sd, rd.argument);
		else if(!strcmp(rd.command,"quit"))
			quit();
	}
}

voi help(){
char* string ={""
"Comandi disponibili\n"
"help:\n"
"--> mostra l’elenco dei comandi disponibili add [file]\n"
"--> invia il file specificato al server remoto compress [alg]\n"
"--> riceve dal server remoto l’archivio compresso secondo l’algoritmo specificato\n"
"quit\n"
"--> disconnessione\n"};
printf(string);
}



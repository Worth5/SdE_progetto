
#include <stdint.h>			//for uint8_t
#include <stdio.h>			//for standard I/O
#include <string.h>			//for strerror()
#include <errno.h>			//for errno
#include <fcntl.h>			//for flags like O_RDONLY and SEEK_END
#include <stdlib.h>			//for exit() and EXIT_FAILURE
#include <unistd.h>			//for unbuffered functions-> read(), write(), STDIN_FILENO
#include <sys/socket.h> 	//for int socket(int domain, int type, int protocol);
#include <arpa/inet.h>		//for uint16_t htons(uint16_t data);


struct request{				//idea creo una struttura che contiene comando e argomento
    char* command;
    char* argument;
};
int setup (int argc, char* argv[]);
request get_request();                    
manage_request(request* myrequest);


int main(int argc, char* argv[]){

	request rq;				//struttura request
	int sd;					//socket descriptor
    sd=setup(argc, argv);   //gestisce la creazione socket e connessione al server
	
    do{
        rq = get_request(); //creo struttura request
        manage_request(&rq,sd);
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
            port_no = atoi(argv[2]); //manca controllo errore.... meglio cmbiare funzione completamente
        }
        else {
            char* addr_str=argv[2]
            port_no = atoi(argv[1]); //manca controllo errore.... meglio cmbiare funzione completamente
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

	// preparazione della struttura contenente indirizzo IP e porta
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port_no);
    sa.sin_addr.s_addr = address;

	//cnnessione al server
	printf("Connessione...\n");
    if (connect(sd, (struct sockaddr*)&sa, sizeof(struct sockaddr_in)) < 0){
        fprintf(stderr, "Impossibile connettersi: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

	return sd;	//ritorno socket descriptor
  }    


request get_request(){
	printf("rcomp> ");
	
}

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

//#define BUFFSIZE 4096
#define PATH_MAX 4096//also defined in <limits.h>
//abilita printf di debug all'interno dell funzione debug()
//#define DEBUG_LEVEL 10 //0  disabilita tutte printf di debug, più è alto il valore maggiori saranno le printf...
#define COMMAND_LENGHT_MAX 9

int sd;			//socket descriptor
struct request{	//creo una struttura che contiene comando e argomento			
	char command[COMMAND_LENGHT_MAX+1];// + \0			
	char argument[PATH_MAX];
};

//funzioni nel main
int setup (int argc, char* argv[]);
void get_request(struct request *rq);                    
void manage_request(int sd, struct request rq);//posso ottimizzare passando come puntatore a costante

//funzioni subordinate
int fget_word(FILE* fd, char* str, int lenght_max);
int parse_argv_for_ip(int argc, char* argv[]);
int parse_argv_for_port(int argc, char* argv[]);

//funzioni accessorie/opzionali
void debug(char *str, int level){	//stampa in verde messaggio debug
	#ifdef DEBUG_LEVEL
		if (DEBUG_LEVEL >= level){
			while((--level)>0)fprintf(stderr,"\t");	//indenta level volte per visibilità
			fprintf(stderr,"\033[0;33m%s\033[1;0m", str); //colore verde \033[1;32m colore normale \033[1;0m
		}
	#endif
}

//funzioni client richieste
void help();
void add(int sd, char* argument);
void compress(int sd, char* argument);
void quit();

//signal handler
void sigint_handler(int signo) {
	quit();
} 

//--------------- MAIN --------------- //
int main(int argc, char *argv[]){
	debug("\nmain()_start\n",1);

	if(signal(SIGINT, sigint_handler) == SIG_ERR){			//Registrazione handler per il segnale SIGINT
		fprintf(stderr, "ERROR: Handler SIGINT registration failed (%s)\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	sd = setup(argc, argv); // gestisce la creazione socket e connessione al server
	struct request rq;
	
	while(1){//up to manage requets to call quit

		debug("main()_while\n",2);
		get_request(&rq); 		// fill struct with command and argument
		manage_request(sd, rq);	// controllo contenuto request e chiamo la funzione principale corrispondente
	}

	debug("main()_non dovresti essere qui\n",1);
					
}

////////////////////////////////////////////////

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

/////////////////////////////////////////////////

int parse_argv_for_ip(int argc, char* argv[]){
	debug("parse_argv_for_ip()\n",3);
	int byte[4];
	for(int j = 1; j < argc; j++){
		if(strlen(argv[j])>16){//attenzione sizeof no
			return -1;
		}
		if(sscanf(argv[j],"%d.%d.%d.%d%s",&byte[0],&byte[1],&byte[2],&byte[3])==4){//controllo quale argomento ha formato ip
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

//////////////////////////////////////////////////

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

//////////////////////////////////////////////////

void get_request(struct request *rq){

	printf("rcomp> ");

	//prendo due parole da stdin poi svuoto stdin se fosse rimasto qualcosa
	debug("get_request()_get_word_1\n",3);	

	//fgets();
	//sscanf();

	int ret = fget_word(stdin, rq->command,COMMAND_LENGHT_MAX); 	//prima parola

	if((ret != '\n') && (ret != '\0')){

		debug("get_request()_get_word_2\n",3);
		ret = fget_word(stdin, rq->argument, PATH_MAX);	//seconda parola

	}
	else{
		rq->argument[0]='\0';
	}
  
	if((ret != '\n') && (ret != '\0')){
		debug("get_request()_flush_exess_stdin\n",4);
		int temp;
		while (((temp = getchar()) != '\n') && (temp != EOF))	//svuoto stdin
			debug((char*)&temp,4);
	}

	printf("Command: '%s'\n", rq->command);
	printf("Argument: '%s'\n", rq->argument);
	
}

//////////////////////////////////////////////////

int fget_word(FILE* fd, char* str,int lenght_max){
	int byte;
	int SIZE = 2;
	str[0]='\0';
	debug("fget_word()_remove_spaces\n",4);
	while(((byte = fgetc(fd)) == ' ')){	//remove leading white spaces
	}
	debug("fget_word()_ungetc()\n",4);
	ungetc(byte, fd);	//rimette nello stream il primo non_space character

	int cont=0;
	debug("fget_word()_get_letters\n",4);

	while(((byte = fgetc(fd)) != '\n') && (byte != ' ') && (byte !=EOF)){

		if(cont > lenght_max){
			fprintf(stderr,"input discarded, max lenght exeeded\n");
			str[0]= (int)'\0';
			return '\0'; //oltre lunghezza max
		}
		str[cont++]=(char)byte;
		//debug((char*)&byte,5);
		//debug("\n",0);
	}

	if (ferror(fd)||(cont==0)){
		debug("fget_word()_error returned empty string",4);
		return str[0]='\0';
	}
	else{
		str[cont]='\0';
	}
	return byte;
}

//////////////////////////////////////////////////

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
	else if(strcmp(rq.command,"quit") == 0){
		quit();
	}
	else{	
		printf("ERROR: unknown command\n");
	}
}

//////////////////////////////////////////////////

void help(){
	debug("help()\n",4);
	const char* string =
	    "Comandi disponibili:\n"
	    "\thelp\n"
	    "\t --> mostra l’elenco dei comandi disponibili add [file]\n"
		"\tadd\n"
	    "\t --> invia il file specificato al server remoto compress [alg]\n"
		"\tcompress\n"
	    "\t --> riceve dal server remoto l’archivio compresso secondo l’algoritmo specificato\n"
	    "\tquit\n"
	    "\t --> disconnessione\n";
	printf(string);
}

//////////////////////////////////////////////////

void add(int sd, char* argument){
	debug("add()_argument:",4);debug(argument,0);debug("\n",0);
	
	for(int i = 0; argument[i] != '\0'; i++){
		if((argument[i] < 'A' || argument[i] > 'Z') && (argument[i] < 'a' || argument[i] > 'z') && (argument[i] < '0' || argument[i] > '9') && argument[i] != '.'){
			fprintf(stderr, "ERROR: Invalid file name\n");
			return;
		}
	}

	int fd;
	if((fd = open(argument, O_RDONLY)) < 0){
		fprintf(stderr, "ERROR: File '%s' not found: (%s)\n", argument, strerror(errno));
		return;
	}

	struct stat metadata;
	if(stat(argument, &metadata) < 0){
		fprintf(stderr, "ERROR: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	off_t file_size = metadata.st_size;			//ricavo dimensione del file da inviare

	int permissions;							//ricavo i permessi del file da inviare
	if(S_ISREG(metadata.st_mode) < 0){
		printf("Error: '%s' is not a regular file (could be a Directory)\n", argument);
		return;
	}
	permissions = metadata.st_mode&0777;

	//-----------------INVIO COMANDO AL SERVER---------------------//
	ssize_t snd_bytes;
	if((snd_bytes = send(sd, "a", 1, 0)) < 0){
		fprintf(stderr, "ERROR: Impossible to send data on socket (%s)\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	//il server si preoccupa di aggiungeere \0 alla fine

	//------------INVIO LUNGHEZZA "FILE NAME"-----------------//
	size_t name_len = htonl(strlen(argument));
	if((snd_bytes = send(sd, &name_len, sizeof(int), 0)) < 0){
		fprintf(stderr, "ERROR: Impossible to send data on socket (%s)\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	//------------------INVIO "FILE NAME"----------------------//
	if((snd_bytes = send(sd, argument, strlen(argument), 0)) < 0){
		fprintf(stderr, "ERROR: Impossible to send data on socket (%s)\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	//il server si preoccupa di aggiungeere \0 alla fine

	//-------------------INVIO PERMESSI------------------------//
	permissions = htonl(permissions);
	if((snd_bytes = send(sd, &permissions, sizeof(mode_t), 0)) < 0){
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

//////////////////////////////////////////////////

void compress(int sd, char* argument){
	debug("compress()_argument:\n",4);				//debug se printa sei nella funzione
	debug(argument,0);debug("\n",0);
	ssize_t rcvd_bytes, snd_bytes;

	if (strcmp(argument, "z") == 0 || strcmp(argument, "j") == 0){ 
           if ((snd_bytes = send(sd, argument, strlen(argument), 0)) < 0){ // Dico al server quale algoritmo usare
			fprintf(stderr, "Error : Unable to specify the algorithm: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
	   }
		printf("Messaggio inviato al server: %s\n", argument);
	}else if (strcmp(argument, "") == 0){
		if ((snd_bytes = send(sd, "z", 1, 0)) < 0) {
			fprintf(stderr, "Error : Unable to specify the algorithm: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		printf("Message sent to the server: %s (Empty argument, set to 'z')\n", argument);
	}else{
		printf("Error: Invalid argument, use 'z' for gzip or 'j' for bzip2.\n");
		return ;
	}

	// --- RICEZIONE RISPOSTA --- //
	char resp[3];
	rcvd_bytes = recv(sd, &resp, 2, 0);
	if (rcvd_bytes < 0){
		fprintf(stderr, "ERRORE: impossibile ricevere dati: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	resp[3] = '\0';
	printf("risposta: %s\n",resp);

	char filename[30];
	if (strcmp(resp, "OK") == 0) {
		if (strcmp(argument, "z") == 0){
			strcpy(filename,"archiviocompresso.tar.gz");
		}else{
			strcpy(filename,"archiviocompresso.tar.bz2");
		}
		printf("Server replied with  'OK'\n -> Ready to receive: '%s'\n",filename);
	}else if(strcmp(resp, "NO") == 0) {
		printf("Server responded with 'NO'\n -> Add at least one file before compression.\n");
		return;
	}else {
		fprintf(stderr, "ERROR: Unexpected response from the server: %s\n", resp);
		exit(EXIT_FAILURE);
	}

	FILE *myfile = fopen(filename, "w");   //apro file
	if (myfile == NULL) {
		fprintf(stderr, "Errore: Impossibile aprire il file: %s\n",strerror(errno));
			exit(EXIT_FAILURE);
	}
	chmod(filename,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);

	
	int file_size;
        int rcvd_bytes2 = recv(sd, &file_size, sizeof(file_size), 0);
        if (rcvd_bytes2 < 0) {
           fprintf(stderr, "Error receiving file size: %s\n", strerror(errno));
           exit(EXIT_FAILURE);
	}
    file_size = ntohl(file_size);
	printf("Received file size: %d bytes\n", file_size);
	


	const int BUFFSIZE=4096;
	char buffer[BUFFSIZE];
	size_t bytes_read;
	long total_read = 0;
	while ( total_read<file_size) {
		int rcvd_bytes3 = recv(sd, &buffer, sizeof(buffer), 0);
		size_t bytes_written = fwrite(buffer, 1, rcvd_bytes3, myfile);
		total_read += bytes_written;
	
		if (bytes_written < rcvd_bytes3) {
			fprintf(stderr, "Error writing to temporary file: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
	
	if (fclose(myfile) < 0) {
	   fprintf(stderr, "Error closing the file: %s\n", strerror(errno));
           exit(EXIT_FAILURE);
	}
    
	printf("File received: %s\n", filename);

}

//////////////////////////////////////////////////

void quit() {
	printf("\nQuitting\n");
	//debug("quit()\n",4);	//debug se printa sei nella funzione

	if (send(sd, "q", 1, 0) < 0){	//invio quit al server
		fprintf(stderr, "Impossibile inviare dati su socket: %s\n", strerror(errno));
		//exit(EXIT_FAILURE); non serve uscire qui sto comunque pianificando di farlo..
	} 

	close(sd);

	exit(EXIT_SUCCESS);
}







#include <stdint.h>	//for uint8_t
#include <stdio.h>	//for standard I/O
#include <string.h>	//for strerror()
#include <errno.h>	//for errno
#include <fcntl.h>	//for flags like O_RDONLY and SEEK_END
#include <stdlib.h>	//for exit() and EXIT_FAILURE
#include <unistd.h>	//for unbuffered functions-> read(), write(), STDIN_FILENO
#include <sys/socket.h> //for int socket(int domain, int type, int protocol);

//idea creo una struttura che contiene comando e argomento
/*
struct request{
    char* command
    char* argument
};
*/
request get_request();    //qui è dichiarata la funzione
manage_request(request* myrequest);

int main(int argc, char* argv){
    setup(address, port);   //gestisce la connessione
                            //probabilmente la funzione deve restituire un socket?
    do{
        request = myrequest; //creo struttura request
        myrequest = get_request();
        manage_request(&request);   //probabilmente a manage request passato anche il socket
    }
    while(strcomp(command,"quit"));
}


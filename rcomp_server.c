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

#define BUFFSIZE 4096
#define R 0
#define W 1

const char dir_name[30] = strcat("rcomp_server.temp.", itoa(getpid()));
//pid 22200
//str rcomp.temp.2200

// funzioni main
int setup(int argc, char *argv[]);
int connect_client(int sd);

// funzioni server richieste
void add(int conn_sd);
void compress(int conn_sd, char *compress_type);

// funzioni subordinate
int parse_argv_for_port(int argc, char *argv[]);
long get_dir_size(char *dir_path);
pid_t new_child_std_to_pipe(char **process, int *child_std, int mode);
long read_from_write_to(FILE *s_input, FILE *s_output, long size_to_write, char *progress_bar_message);

//funzioni opzionali

int main(int argc, char *argv[]) {
    int sd = setup(argc, argv);  // crea socket imposta sockopt esegue bind e listen

    while (1) {
        int conn_sd = connect_client(sd);  // esegue connet e restituisce connection socket
        int valid_input = 1;
        char command[10] = "0";

        while (valid_input && strcmp(command, "q")) {  // finche input è valido e diverso da "q"

            if (recv(conn_sd, &command, 1, 0) < 0) {
                fprintf(stderr, "Impossibile ricevere dati da socket: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }

            if (!strcmp(command, "a")) {
                add(conn_sd);  // riceve file e lo salva
            }
            else if ((!strcmp(command, "j")) || (!strcmp(command, "z"))) {
                compress(conn_sd, command);  // fork al comando tar con exec poi manda il file al client
            }
            else if (!strcmp(command, "q")) {
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
    struct stat dir_info;
    mode_t mode = S_IRUSR | S_IWUSR | S_IXUSR;
    if (stat(dir_name, &dirInfo) == -1) {
        if (mkdir(dir_name, mode) < 0) {
            fprintf(stderr, "Error creating directory: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    // sposto working directory in dir_name
    chdir(dir_name);

    // ricevi lunghezza nome del file
    int file_name_len;
    if (recv(conn_sd, &file_name_len, sizeof(int), 0) < 0) {
        fprintf(stderr, "Error receiving size of file name: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    int file_name_len = ntohl(file_name_len);

    // alloco spazio in memoria e ricevo stringa nome del file
    char *filename = malloc(file_name_len);
    if (recv(conn_sd, &filename, file_name_len, 0) < 0) {
        fprintf(stderr, "Error receiving file name: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // check file gìà presente in questo caso non si fa niente e verrà sovrascritto
    struct stat file_info;
    if (stat(filename, &file_info) == 0) {
        Printf("File alreaddy exists, overwriting with new file\n");
    }

    // crea file all'interno della dir con nome ricevuto
    int fd;
    if ((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 777)) < 0) {//   rwx rwx rwx -> 111 111 111-> 7 7 7 
        fprintf(stderr, "ERROR: cannot create file %s (%s)\n", filename, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // ricevi grandezza file
    int file_size;
    ssize_t rcvd_bytes = recv(conn_sd, &file_size, sizeof(int), 0);
    if (rcvd_bytes < 0) {
        fprintf(stderr, "Error receiving file size: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    int file_size = ntohl(file_size);

    // ciclo recv from socket, write on file
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
    chdir("..");  // gestione errore?
}

//////////////////////////////////////////////////////////////////

void compress(int conn_sd, char *compress_type) {
    printf("Compress request received\n");

    char *tar_command[] = {"tar", "cf", "-", "-C", dir_name, ".", NULL};
    //.. genitore
    //. cartella stessa
    // execvp(tar_command[0],tar_command);
    char *compress_command[] = {(strcmp(compress_type, "j") ? "gzip" : "bzip2"), "-c", NULL};

    long dir_size = get_dir_size(dir_name);
    printf("uncompressed_size: %ld\n", dir_size);

    // send ok or no
    
    char *str = "ok";
    if (dir_size < 0) {
        fprintf(stderr, "No file to compress\n");
        strcpy(str, "no");
        if (send(conn_sd, str, strlen(str) + 1, 0) < 0) {
            fprintf(stderr, "Error sending data: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        return;
    }
    if (send(conn_sd, str, strlen(str) + 1, 0) < 0) {
        fprintf(stderr, "Error sending data: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    /*
    crei figlio (tar-archivio)  -> file oppure pipe + barra progresso -> nuovo processo figlio(copress)-> pipe -> client

    crei figlio (tar e compress) ->pipe -> client
                    ^ bzip2
    */


    // create child redirect std (in e/o out e/o err a seconda di pipe_mode); 
	//di fatto se manualmente non leggi da pipe sdterr figlio aver fatto la redirezione è inutile
	//ma almeno così è già implementato se ci fosse necessità
    int tar_child_pipe[3];//i tre fd per le pipe, creati tutti usati solo quelli specificati
    int tar_pipe_mode = (1 << STDOUT_FILENO) | (1 << STDERR_FILENO);
    pid_t tar_child_pid = new_child_std_to_pipe(tar_command, tar_child_pipe, tar_pipe_mode);

    int compress_child_pipe[3];
    int compress_pipe_mode = (1 << STDIN_FILENO) | (1 << STDOUT_FILENO);
    pid_t compress_child_pid = new_child_std_to_pipe(compress_command, compress_child_pipe, compress_pipe_mode);

    FILE *tar_out_stream = fdopen(tar_child_pipe[STDOUT_FILENO], "r");
    FILE *compress_in_stream = fdopen(compress_child_pipe[STDIN_FILENO], "w");
    FILE *compress_out_stream = fdopen(compress_child_pipe[STDOUT_FILENO], "r");
    if(offline)FILE *myfile = fopen("pippo", "w");  // for debug without client

    /*   read_from_write_to è ciclo read write + controllo errore + barra progresso
    while(fread(tar)>0){
        fwrite(compress);
    }
    */

    long compressed_size = read_from_write_to(tar_out_stream, compress_in_stream, extimate_archive_size(dir_size), "Compression");
    printf("compressed_size: %ld\n", compressed_size);
    if (compressed_size < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

	fclose(tar_out_stream);
	fclose(compress_in_stream);	//questo importantissimo metterlo qui
							//quando chiudo file processi figlio capisce che input è terminato
							// diverso da fare fflush
							//non testato ma prob viene inviato EOF o fa errore la lettura e smette di leggere....
							//ci sarebbe da provare a inviare un EOF quando hi finito tutto quello che va inviato e vedere
							//se si comporta allo stesso mdo che chiudere stream


        // mando lunghezza file
        if (send(conn_sd, hton(compressed_size), sizeof(compressed_size), 0) < 0) {
            fprintf(stderr, "Error sending data: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        // mando file
		long total_sent=0;
        while ((bytes_read = fread(buff, 1, sizeof(buff), compress_out_stream)) > 0) {
            ssize_t snd_bytes = send(conn_sd, buff, bytes_read, 0);
			total_sent+=snd_bytes;
			progress_bar(total_sent,compressed_size,"Sending");
            if (snd_bytes < 0) {
                fprintf(stderr, "Error sending compressed data: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
    // mancano i controlli errore
    close(tar_child_pipe[STDERR_FILENO]);
    close(tar_child_pipe[STDIN_FILENO]);
    close(compress_child_pipe[STDERR_FILENO]);

    fclose(compress_out_stream);

/////////////////////////////////////////////////////////////////////////////

pid_t new_child_std_to_pipe(char **process, int *child_std, int mode) {
    /*
    la funzione crea tre pipe le associa a std in out err del processo figlio
    il processo figlio è creato con execl(command , argument)
    ritorna pid processo figlio
    */

    int p_input[2];
    int p_output[2];
    int p_err[2];
    pipe(p_input);
    pipe(p_output);
    pipe(p_err);

    // ---------- fork -------------//
    pid_t PID_CHILD = fork();
    if (PID_CHILD == -1) {
        fprintf(stderr, "Error executing fork: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    //--------- child --------------//
    if (PID_CHILD == 0) {
        close(p_input[W]);
        close(p_output[R]);
        close(p_output[R]);

        if (mode & (1 << STDIN_FILENO)) {
            dup2(p_input[R], STDIN_FILENO);
        }
        close(p_input[R]);

        if (mode & (1 << STDOUT_FILENO)) {
            dup2(p_output[W], STDOUT_FILENO);
        }
        close(p_output[W]);

        if (mode & (1 << STDERR_FILENO)) {
            dup2(p_err[W], STDERR_FILENO);
        }
        close(p_err[W]);

        if (mode & (1 << STDERR_FILENO)) {  // debug
            fprintf(stderr, "child speaking\n");
        }

        // execlp("sh", "sh", "-c", process, (char *)NULL);
        execvp(process[0], process);
        fprintf(stderr, "Error executing child: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    //--------- parent --------------//
    close(p_input[R]);
    close(p_output[W]);
    close(p_err[W]);

    // passo pipe richieste chiudo le altre
    if (mode & (1 << STDIN_FILENO)) {
        child_std[STDIN_FILENO] = p_input[W];
        printf("child %d: stdin redirected\n", PID_CHILD);
    }
    else {
        close(p_input[W]);
    }

    if (mode & (1 << STDOUT_FILENO)) {
        child_std[STDOUT_FILENO] = p_output[R];
        printf("child %d: stdout redirected\n", PID_CHILD);
    }
    else {
        close(p_output[R]);
    }

    if (mode & (1 << STDERR_FILENO)) {
        child_std[STDERR_FILENO] = p_err[R];
        printf("child %d: stdout redirected\n", PID_CHILD);
    }
    else {
        close(p_err[R]);
    }

    return PID_CHILD;
}

////////////////////////////////////////////////////

long read_from_write_to(FILE *s_input, FILE *s_output, long total_to_write, char *progress_bar_message) {
    // creo archivio e lo salvo in file temp

    char buffer[BUFFSIZE];
    size_t bytes_read;
    long total_read = 0;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), s_input)) > 0) {
        size_t bytes_written = fwrite(buffer, 1, bytes_read, s_output);
        total_read += bytes_written;
        if (bytes_written < bytes_read) {
            fprintf(stderr, "ERROR accessing file: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (total_to_write) {
            progress_bar(total_read, total_to_write, progress_bar_message);
        }
    }
    //altrimenti feof()
    if (ferror(s_input)) {
        fprintf(stderr, "ERROR accessing file: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (size_to_write) {
        printf("\n");  // per avere \n dopo progress bar
    }
    // return filesize;
    return total_read;
}

///////////////////////////////////////////////

long get_dir_size(char *dir_path) {
    // controllo che la cartella esista
    struct stat dir_stat;
    if (stat(dir_path, &dir_stat) == -1) {
        // dir non esiste -> nessun file da comprimere
        if (errno == ENOENT) {  // ENOENT 
            return -1;
        }
        else {  // errore diverso in lettura dir
            fprintf(stderr, "ERROR: cannot open dir %s (%s)\n", dir_path, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    DIR *dp = opendir(dir_path);
    if (dp == NULL) {
        printf("impossbile aprire directory%S: %s", dir_path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    struct dirent *dirp;  // dirent = dir entries
    //dirent ->[puntatore a i-node, nomefile]
    struct stat file_stat;
    long total_size = 0;
    //cartella
        //dirent 1
        //dirent 2
        //dirent n

    // raddir()  -> dirent numero 1 ->[puntatore a i-node, nomefile]
    // readdir() -> dirent nomero 2 ->

    while ((dirp = readdir(dp)) != NULL) {

        //prendo nome file
        char file_path[PATH_MAX];
        snprintf(file_path, sizeof(file_path), "%s/%s", dir_path, dirp->d_name);

        //uso il nome del file nella stat()
        if (stat(file_path, &file_stat) == -1) {
            fprintf(stderr, "Error getting file metadata:%s", dirp->d_name);
            closedir(dp);
            return -1;
        }

        if (S_ISREG(file_stat.st_mode)) {
            // Regular file, not a directory
            total_size += file_stat.st_size;
        }
        else if (S_ISDIR(file_stat.st_mode)) {
            // here recursion if u want function to search subfolers
            // get_dir_size(thisfolder)
        }
    }

    closedir(dp);
    return total_size;
}

///////////////////////////////////////////////

void progress_bar(long processed, long total, char *message) {
    printf("Processed= %ld    ", processed);
    int progress = (int)((processed / (float)total) * 100);
    printf("%s progress: [%d%%]\r", message, progress);//  %d per intero  %% per scrivere "%"  -> "message progress:[n%]
    fflush(stdout);
}

///////////////////////////////////////////////7

long extimate_archive_size(long dir_size) {  // ricavata a tentativi per ora corretta
    long expected_size = dir_size - (dir_size % 10240) + 10240;
    if ((dir_size % 10240) <= 8192)
        return expected_size;
    else
        return expected_size + 10240;
}



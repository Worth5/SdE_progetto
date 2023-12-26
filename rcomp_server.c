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

const char dir_name[30] = strcat("rcomp_server.temp.", atoi(getpid()));

// funzioni main
int setup(int argc, char *argv[]);
int connect_client(int sd);

// funzioni server richieste
void add(int conn_sd);
void compress(int conn_sd);

// funzioni subordinate
int parse_argv_for_port(int argc, char *argv[]);
long get_dir_size(char *dir_path);

// funzioni opzionali
void compress2(int conn_sd);                                                                               // sostituisce la compress semplice aggiungendo progress bar
long tar_size = build_archive(FILE * temp_archive, char *dir_name, long dir_size);                         // richiesta incompress2
long zip_size = build_zip(FILE * temp_zip, FILE *temp_archive, char *compression_command, long tar_size);  // richiesta in compress2

int main(int argc, char *argv[]) {
    int sd = setup(argc, argv);  // crea socket imposta sockopt esegue bind e listen

    while (1) {
        int conn_sd = connect_client(sd);  // esegue connet e restituisce connection socket
        int valid_input = 1;
        char command[10] = "0";

        while (valid_input && strcmp(command, "q")) {  // finche input è valido e diverso da "q"

            ssize_t rcvd_bytes = recv(conn_sd, &command, 1, 0);

            if (rcvd_bytes < 0) {
                fprintf(stderr, "Impossibile ricevere dati su socket: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            else if (!strcmp(command, "a")) {
                add(conn_sd);  // riceve file e lo salva
            }
            else if ((!strcmp(command, "j")) || (!strcmp(command, "z"))) {
                compress(conn_sd);  // fork al comando tar con exec poi manda il file al client
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
    if ((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0777)) < 0) {
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

void compress(int conn_sd) {
    printf("Compress request received\n");

    // receive compression type from client
    char arg[2];
    ssize_t rcvd_bytes;
    if ((rcvd_bytes = recv(conn_sd, arg, sizeof(arg), 0)) < 0) {
        fprintf(stderr, "Error receiving compression type: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // setup tar command
    char tar_command[100];
    if (strcmp(arg, "z") == 0) {
        snprintf(tar_command, sizeof(tar_command), "tar czf - -C ./%s .", dir_name);
    }
    else if (strcmp(arg, "j") == 0) {
        snprintf(tar_command, sizeof(tar_command), "tar cjf - -C ./%s .", dir_name);
    }
    else {
        fprintf(stderr, "Invalid compression type received: %s\n", arg);
        exit(EXIT_FAILURE);
    }

    // controllo che la cartella esista
    struct stat directory_stat;
    if (stat(dir_name, &dir_stat) == -1) {
        // dir non esiste -> nessun file da comprimere
        if (errno == ENOENT) {  // ENOENT
            char *str = "no";
            if (send(conn_sd, str, strlen(str) + 1, 0) < 0) {
                fprintf(stderr, "Error sending data: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            printf("No file ready for compression");
            return;
        }
        else {  // errore diverso in lettura dir
            fprintf(stderr, "ERROR: cannot open dir %s (%s)\n", filename, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    // ok to server comincia compressione
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

    // ho bisogno di mandare lunghezza file
    // copio archivio in file temporaneo e ne valuto la lunghezza
    FILE *temp_file = tmpfile();
    if (temp_file == NULL) {
        fprintf(stderr, "Error creating temporary file: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    char buff[BUFFSIZE];
    size_t bytes_read = 1;
    while (bytes_read > 0) {
        if (bytes_read = fread(buffer, 1, sizeof(buffer), tarStream) < 0) {
            fprintf(stderr, "Error reading child process: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (fwrite(buffer, 1, bytes_read, temp_file) < 0) {
            fprintf(stderr, "Error wrinting temp file: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    // trovo filesize
    fseek(temp_file, 0, SEEK_END);
    long fileSize = ftell(tempFile);
    fseek(temp_file, 0, SEEK_SET);

    // comunico filesize
    if (send(conn_sd, hton(fileSize), sizeof(fileSize), 0); < 0) {
        fprintf(stderr, "Error sending compressed data: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // legge stream temp_file e invia bytes a client
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

//////////////////////////////////////////////////////////////

void compress2(int conn_sd) {
    // receive compression type from client
    char arg[2];
    ssize_t rcvd_bytes;
    if ((rcvd_bytes = recv(conn_sd, arg, sizeof(arg), 0)) < 0) {
        fprintf(stderr, "Error receiving compression type: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    char compression_command[10];
    if (strcmp(arg, "z") == 0) {
        snprintf(compression_command, sizeof(compression_command), "gzip", dir_name);
    }
    else if (strcmp(arg, "j") == 0) {
        snprintf(compression_command, sizeof(compression_command), "bzip2", dir_name);
    }
    else {
        return -1;
    }

    // ottengo grandezza dir e controllo allo stesso tempo se ci sono file disponibili per compress
    long dir_size;
    if ((dir_size = get_dir_size(dir_name)) < 0) {
        char *str = "no";
        if (send(conn_sd, str, strlen(str) + 1, 0) < 0) {
            fprintf(stderr, "Error sending data: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        printf("No file ready for compression\n");
    }

    // creo file temporaneo e ci salvo archivio
    FILE *temp_archive = tmpfile();
    if (temp_archive == NULL) {
        fprintf(stderr, "Error creating temporary file: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    // creo archivio e ritorno grandezza in byte
    long tar_size = build_archive(temp_archive, dir_name, dir_size);

    // creo file temporaneo e ci salvo file compresso
    FILE *temp_zip = tmpfile();
    if (temp_archive == NULL) {
        fprintf(stderr, "Error creating temporary file: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    // creo file compresso e ritorno grandezza in byte
    long zip_size = build_zip(temp_zip, temp_archive, compression_command, tar_size);

    // comunico filesize
    if (send(conn_sd, hton(fileSize), sizeof(fileSize), 0); < 0) {
        fprintf(stderr, "Error sending compressed data: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // legge stream temp_file e invia bytes a client
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
}

//////////////////////////////////////////////////////////////

long build_archive(FILE *temp_file, char *dir_name, int dir_size) {
    // creo stream associato a comando tar con popen()
    char tar_command[50];
    snprintf(tar_command, sizeof(tar_command), "tar cf - -C ./%s .", dir_name);
    FILE *tar_stream = popen(tar_command, "r");
    if (tar_stream == NULL) {
        fprintf(stderr, "Error executing tar: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // creo archivio e lo salvo in file temp
    char buffer[BUFFSIZE];
    size_t bytes_read;
    long total_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), tar_stream)) > 0) {
        if (fwrite(buffer, 1, bytes_read, temp_file) < bytes_read) {
            fprintf(stderr, "Error writing to temporary file: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        else {
            // qui implemento una progress bar
            total_read += bytes_read;
            int progress = (int)((total_read / (float)dir_size) * 100);
            printf("Progress: [%d%%]\r", progress);
            fflush(stdout);
        }
    }

    if (ferror(tar_stream)) {
        fprintf(stderr, "Error reding from archive: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    else {
        printf("Progress: [100%%]\n");  // per avere \n
    }

    /* altro modo per trovare lunghezza
    //trovo filesize file temporaneo
    fseek(temp_file, 0, SEEK_END);
    long fileSize = ftell(tempFile);
    fseek(temp_file, 0, SEEK_SET);
    */
    close(tar_stream);
    // return filesize;
    return total_read;
}

//////////////////////////////////////////////////////////////

long build_zip(FILE *temp_zip, FILE *temp_archive, char *command, long tar_size) {
    // creo processo figlio con pipe input e pipe output
    // prende archivio in input
    // lo comprime in output
    int p_input[2];
    int p_output[2];
    pipe(p_input);
    pipe(p_output);

    // fork
    pid_t PID_child_compress = fork();
    if (PID_child_compress == -1) {
        fprintf(stderr, "Error executing fork: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (PID_child_compress == 0) {
        dup2(p_input[R], STDIN_FILENO);
        close(p_input[W]);
        dup2(p_output[R], STDOUT_FILENO);
        close(p_output[W]);

        execlp(command, command, "-c", NULL);
        fprintf(stderr, "Error executing child: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    else {
        close(p_input[R]);
        close(p_output[W]);
    }

    FILE *to_input = fdopen(p_input[1], "w");
    FILE *from_output = fdopen(p_output[0], "r");

    // write to compression algorithm
    char buffer[BUFFSIZE];
    size_t bytes_read = 1;
    while (bytes_read = fread(buffer, 1, sizeof(buffer), temp_archive) > 0) {
        if (fwrite(buffer, 1, bytes_read, to_input) < 0) {
            fprintf(stderr, "Error wrinting temp file: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        else {
            // qui implemento una progress bar
            total_read += bytes_read;
            int progress = (int)((total_read / (float)tar_size) * 100);
            printf("Progress: [%d%%]\r", progress);
            fflush(stdout);
        }
    }
    fclose(to_input);  // esegue flush input buffer

    // read from compression algorithm and store file
    while (bytes_read = fread(buffer, 1, sizeof(buffer), from_output) > 0) {
        if (fwrite(buffer, 1, bytes_read, temp_zip) < 0) {
            fprintf(stderr, "Error wrinting temp file: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    printf("Progress: [100%%]\n");  // per avere \n
    fclose(from_output);
}

//////////////////////////////////////////////////////////////

long get_dir_size(char *dir_path) {
    // controllo che la cartella esista
    struct stat dir_stat;
    if (stat(dir_name, &dir_stat) == -1) {
        // dir non esiste -> nessun file da comprimere
        if (errno == ENOENT) {  // ENOENT
            return -1;
        }
        else {  // errore diverso in lettura dir
            fprintf(stderr, "ERROR: cannot open dir %s (%s)\n", dir_name, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    DIR *dp = opendir(dir_path);
    struct dirent *dirp;  // dirent = dir entries
    struct stat file_stat;
    long total_size = 0;

    while ((dirp = readdir(dp)) != NULL) {
        // ottieni dimensione del file
        if (stat(dirp->d_name, &file_stat) < 0) {
            fprintf(stderr, "Impossibile recuperare metadati del file %s\n", dirp->d_name);
            exit(EXIT_FAILURE);
        }
        if (strcmp(dirp->d_name, "..") != 0) {
            total_size += file_stat.st_size;
        }
    }

    closedir(dp);
    return total_size;
}

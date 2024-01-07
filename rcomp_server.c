
#include <arpa/inet.h>   //for inet_pton() INADDR_ANY credo...
#include <dirent.h>      //for struct dirent()
#include <errno.h>       //for errno
#include <fcntl.h>       //for flags like O_RDONLY and SEEK_END
#include <stdio.h>       //for standard I/O
#include <stdlib.h>      //for exit() and EXIT_FAILURE
#include <string.h>      //for strerror()
#include <sys/socket.h>  //for socket
#include <sys/stat.h>    //for stat()
#include <sys/wait.h>    //for wait()
#include <unistd.h>      //for unbuffered functions-> read(), wriqte(), STDIN_FILENO

//-----------------------------------------

#define BUFFSIZE 4096
#define PATH_MAX 4096

int sd;  // globale per poterlo chiudere da exit handler

char tempFolder[30];                      // parent, name assigned on runtime
char recvFolder[] = "rcvdFiles";          // inside parent, contains received files
char compressedFile[] = "compress.temp";  // inside parent, compressed output file
// avvio server crea cartella temporanea "parent"
// alla ricezione file li mette nella sottocartella "rcvdFiles"
// alla compressione genera file compresso "compress.temp" e elimino "rcvdFiles"
// alla chiusura con quit() elimino la cartella parent
// per evitare che la cartella rimanga usiamo un exit handler

//---------------------------------------

// funzioni main
int setup(int port);
int accept_client(int sd);
void quit();


// /signal-exit handler
void sigint_handler(int signo) {exit(130);}//script teminated by ^C code
void exit_handler(){quit();}

// funzioni server richieste
void add(int conn_sd);
void compress(int conn_sd, char *compress_type);

// funzioni subordinate
int parse_argv_for_port(int argc, char *argv[]);
int get_size(char *path);  // return size of file or folder(all files contained) return -1 on file missing
int fread_from_fwrite_to(FILE *s_input, FILE *s_output, int size_to_write, char *progress_bar_message);

//----------------------------------------

// funzioni opzionali non parte progettino
void progress_bar(int processed, int total, char *message);
int extimate_archive_size(int dir_size);

// funzioni debug non parte progettino
#define DEBUG  // remove if u dont need debug
               //if removed functions bolow do nothing
void parse_arg_for_debug_option(int argc, char *argv[]);
int debug(const char *str, ...);


/////////////////////--------------------INIZIO MAIN--------------------////////////////////////

int main(int argc, char *argv[]) {
    parse_arg_for_debug_option(argc, argv);  // if -d or -v specified enable debug print
    int port = parse_argv_for_port(argc, argv);
    if (port < 0){
		port = 1234;
		printf("No valid port, using default: %d\n", port);
	}

    debug("\nmain()_start\n");
    snprintf(tempFolder, 30, "rcomp_server_temp_%d", getpid());

    debug("main()_create_temp_folder\n");
    mode_t mode = S_IRWXU; // = (S_IRUSR | S_IWUSR | S_IXUSR)
    if (mkdir(tempFolder, mode) < 0) {
        fprintf(stderr, "ERROR creating directory: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    chdir(tempFolder);

    if (atexit(exit_handler)!=0) {  // Registrazione exit handler
        fprintf(stderr, "ERROR exit handler registration failed (%s)\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
	if(signal(SIGINT, sigint_handler) == SIG_ERR){			//Registrazione handler per il segnale SIGINT
		fprintf(stderr, "ERROR: Handler SIGINT registration failed (%s)\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

    sd = setup(port);  // crea socket imposta sockopt esegue bind e listen

    while (1) {
        debug("+main()_manage clients\n");
        int conn_sd = accept_client(sd);  // esegue connet e restituisce connection socket
        int valid_input = 1;
        char command[10] = "0";

        while (valid_input && (strcmp(command, "q") != 0)) {  // finche input è valido e diverso da "q"
            debug("+while()_manage_requests\n");
			printf("- Waiting for request -\n");

            if (recv(conn_sd, &command, 1, 0) < 0) {
                fprintf(stderr, "Impossibile ricevere dati da socket: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            command[1] = '\0';
			printf("Received: %s\n",command);
            debug("request: '%s'\n", command);

            if (strcmp(command, "a") == 0) {
                add(conn_sd);  // riceve file e lo salva
            }
            else if ((strcmp(command, "j") == 0) || (strcmp(command, "z") == 0)) {
                compress(conn_sd, command);  // fork al comando tar con exec poi manda il file al client
            }
            else if (strcmp(command, "q") == 0) {
                printf("Connection interrupted by client\n");
				break;
            }
            else {
                valid_input = 0;
                printf("Invalid command detected(%s)\n", command);
            }
            debug("-");
        }
        printf("Closing connection\n");
        close(conn_sd);
        debug("-");
    }

    debug("ERRORE:non dovresti essere qui");
}

//////////////////////---------------------FUNZIONI MAIN------------------------/////////////////////////

int setup(int port) {  // create socket + socket opt + bind() return socket
    debug("+setup()\n");


    int sd;
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "ERROR failed to create socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    char *addr_str = "127.0.0.1";
    in_addr_t address;
    if (inet_pton(AF_INET, addr_str, (void *)&address) < 0) {  // conversione dell'indirizzo in formato numerico
        fprintf(stderr, "ERROR failed to convert ip adress: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in sa;  // preparazione della struttura contenente indirizzo IP e porta
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = address;
    int reuse_opt = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &reuse_opt, sizeof(int));

    // --- BIND --- //
    debug("setup()_bind()\n");
    if (bind(sd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {  // associazione indirizzo a socket
        fprintf(stderr, "ERROR failed to bind ip to socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("Socket %d binded to %s:%d\n", sd, addr_str, port);

    // --- LISTENING --- //
    debug("-setup()_listen()\n");
    if (listen(sd, 10) < 0) {
        fprintf(stderr, "ERROR failed to listen on socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return sd;
}

///---------------------------------------------------

int accept_client(int sd) {
    debug("+accept_client()\n");
    // --- ATTESA DI CONNESSIONE --- //
    printf("--- Waiting for connection ---\n");

    int conn_sd;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    if ((conn_sd = accept(sd, (struct sockaddr *)&client_addr, &client_addr_len)) < 0) {
        fprintf(stderr, "ERROR failed to accept connection on socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    debug("1accept_client()_accepted\n");

    // conversione dell'indirizzo in formato numerico
    char client_addr_str[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_addr_str, INET_ADDRSTRLEN) < 0) {
        fprintf(stderr, "ERROR failed to convert ip address\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("Connected to client %s:%d\n", client_addr_str, ntohs(client_addr.sin_port));
    debug("-");
    return conn_sd;
}

///---------------------------------------------------

void quit() {
	printf("\nQuitting\n");
    debug("0quit()\n");
    close(sd);  // close
    chdir("..");

    int len = sizeof(tempFolder) + sizeof("rm -r ") - 1;
    char remove[len];
    snprintf(remove, len, "rm -r %s", tempFolder);
    if(system(remove)<0)
		fprintf(stderr,"ERROR failed to remove temp folder\n");
	
}

///////////////////------------------FUNZIONI RICHIESTE PROGETTINO---------------------//////////////////////

void add(int conn_sd) {
    debug("+add()\n");
    // mkdir se dir non esiste
    debug("add()_mkdir\n");
    mode_t mode = S_IRUSR | S_IWUSR | S_IXUSR;
    if (mkdir(recvFolder, mode) < 0) {
        if (errno != EEXIST) {
            fprintf(stderr, "ERROR creating temp directory: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    // sposto working directory in recvFolder
    chdir(recvFolder);

    // ricevi lunghezza nome del file
    size_t fileNameLen;
    if (recv(conn_sd, &fileNameLen, sizeof(int), 0) < 0) {
        fprintf(stderr, "ERROR receiving size of file name: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    fileNameLen = ntohl(fileNameLen);
    debug("add()_rcv_name_len:%d\n", fileNameLen);

    // ricevo stringa nome del file
    char fileName[fileNameLen];
    if (recv(conn_sd, &fileName, fileNameLen, 0) < 0) {
        fprintf(stderr, "ERROR receiving file name: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    fileName[fileNameLen] = '\0';
    debug("add()_rcv_name:%s\n", fileName);

    // ricevo mode file (permissions and type of file)
    mode_t fileMode;
    int temp;
    if (recv(conn_sd, &temp, sizeof(temp), 0) < 0) {
        fprintf(stderr, "ERROR receiving file mode: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    fileMode = ntohl(temp) & 0777;
    debug("add()_rcv_name:%d\n", fileMode);

    // check file gìà presente in questo caso non si fa niente e verrà sovrascritto
    struct stat file_info;
    if (stat(fileName, &file_info) == 0) {
        printf("File alreaddy exists, overwriting with new file\n");
    }

    // crea file all'interno della dir con nome ricevuto
    int fd;
    if ((fd = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, fileMode)) < 0) {
        fprintf(stderr, "ERROR cannot create file %s (%s)\n", fileName, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // ricevi grandezza file
    int file_size;
    int rcvd_bytes = recv(conn_sd, &file_size, sizeof(file_size), 0);
    if (rcvd_bytes < 0) {
        fprintf(stderr, "ERROR receiving file size: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    file_size = ntohl(file_size);
    debug("add()_rcv_size:%d\n", file_size);

    // loop: recv from socket-> write on file
    char buff[BUFFSIZE];
    int remaining_bytes = file_size;

    while (remaining_bytes > 0) {
        if ((rcvd_bytes = recv(conn_sd, buff, sizeof(buff), 0)) < 0) {
            fprintf(stderr, "ERROR receiving file data: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (write(fd, buff, rcvd_bytes) < 0) {
            fprintf(stderr, "ERROR writing data to file: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        remaining_bytes -= rcvd_bytes;
        debug("add()_while()_bytes_recv:%d\r", file_size - remaining_bytes);
        progress_bar(file_size - remaining_bytes, file_size, "Receiving file");
    }
    debug("-\n");
    printf("\nFile received and saved as: %s\n", fileName);

    // Close the file descriptor
    if(close(fd) < 0) {
        fprintf(stderr, "ERROR closing file descriptor: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if(chdir("..") < 0) {
        fprintf(stderr, "ERROR changing working directory: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

///---------------------------------------------------

void compress(int conn_sd, char *comp_type) {
    debug("+compress()\n");
    // children commands
    char tarCommand[50];
    char compressCommand[50];

    // creo comandi per processi figli
    snprintf(tarCommand, 50, "tar cf - -C %s .", recvFolder);
    if (strcmp(comp_type, "j")) {
        snprintf(compressCommand, 50, "gzip > %s", compressedFile);
    }
    else {
        snprintf(compressCommand, 50, "bzip2 > %s", compressedFile);
    }

    // controllo se ci sono file da comprimere
    // se non ce ne sono ritorno
    int dirSize = get_size(recvFolder);
    if (dirSize < 0) {
        int snd_bytes = send(conn_sd, "NO", 3, 0);  // {'N','O','\0'}
        debug("1compress()_send_NO\n");
        if (snd_bytes < 0) {
            fprintf(stderr, "ERROR sending compressed data: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        return;
    }
    else {
        int snd_bytes = send(conn_sd, "OK", 2, 0);
        debug("1compress()_send_OK\n");
        if (snd_bytes < 0) {
            fprintf(stderr, "ERROR sending compressed data: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    printf("UncompressedSize: %ld\n", dirSize);
    int archiveExtimate = extimate_archive_size(dirSize);  // get extimate of archive size to use for progress bar

    debug("compress()_create children\n");
    // ho separato il comando in due per poter printare una progress bar
    FILE *tarStream = popen(tarCommand, "r");
    FILE *compressStream = popen(compressCommand, "w");
    fread_from_fwrite_to(tarStream, compressStream, archiveExtimate, "Compression");  // qui passo dati da un figlio all'altro e printo progress bar
    fclose(tarStream);
    fclose(compressStream);

    debug("compress()_wait()\n");
    // faccio una wait per aspettare che i processi figli abbiano terminato
    while (wait(NULL) > 0);
    if (errno != ECHILD) {  // errore no figli
        fprintf(stderr, "ERROR on Wait(): %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int compressedSize = get_size(compressedFile);
    printf("Compressed size: %ld\n", compressedSize);
    // mando lunghezza file
    int compressedSize_net = htonl(compressedSize);
    if (send(conn_sd, &compressedSize_net, sizeof(compressedSize_net), 0) < 0) {
        fprintf(stderr, "ERROR sending data: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    debug("compress()_send_compressed_size:%d", compressedSize);

    FILE *zip = fopen(compressedFile, "r");
    // mando file
    char buff[BUFFSIZE];
    int bytes_read, total_sent = 0;
    while ((bytes_read = fread(buff, 1, sizeof(buff), zip)) > 0) {
        ssize_t snd_bytes = send(conn_sd, buff, bytes_read, 0);
        if (snd_bytes < 0) {
            fprintf(stderr, "ERROR sending compressed data: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        total_sent += snd_bytes;
        debug("1compress()_file_total_sent:%s\r", total_sent);
        progress_bar(total_sent, compressedSize, "Sending");
    }
    printf("\n");
    debug("\n");
    fclose(zip);

    debug("-compress()_remove_files");
    // rimuovo cartella con file ricevuti
    printf("Removing uncompressed files\n");
    int len = sizeof(recvFolder) + sizeof("rm -r ") - 1;
    char remove[len];
    snprintf(remove, len, "rm -r %s", recvFolder);
    if(system(remove) < 0) {
		fprintf(stderr, "ERROR removing folder with sent files: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

//////////////////////---------------------FUNZIONI SUBORDINATE------------------------/////////////////////////

int parse_argv_for_port(int argc, char *argv[]) {
    debug("+parse_argv_for_port()\n", 3);
    int port;
    for (int j = 1; j < argc; j++) {
		debug("1%s\n",argv[j]);
        if (sscanf(argv[j], "%d", &port)) {
            if (port >= 0 && port <= 65535) {
				debug("-");
                return port;
            }
        }
    }
	debug("-");
    return -1;
}

///---------------------------------------------------

int get_size(char *path) {
    debug("+get_size()\n");
    // controllo che file esista
    struct stat fileStat;
    if (stat(path, &fileStat) == -1) {
        // file non trovato uso questo ritorno per capire se ci sono file da comprimere
        if (errno == ENOENT) {  // ENOENT
			debug("-");
            return -1;
        }
        else {  // errore diverso in apertura
            fprintf(stderr, "ERROR on path '%s': (%s)\n", path, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    if (S_ISREG(fileStat.st_mode)) {  // se file è file regolare ritorno dimensione
        debug("get_size()_ISREG");
        debug("- size: %d\n", fileStat.st_size);
        return fileStat.st_size;
    }

    else if (S_ISDIR(fileStat.st_mode)) {  // se file è directory scorri elementi
        debug("get_size()_ISDIR\n");
        DIR *dp = opendir(path);
        if (dp == NULL) {
            printf("ERROR failed to open directory '%s': (%s)", path, strerror(errno));
            exit(EXIT_FAILURE);
        }

        // cartella
        // dirent 1
        // dirent 2
        // dirent n
        struct dirent *dirp;  // dirent = dir entries
        struct stat file_stat;
        int total_size = 0;

        // readdir()  -> dirent numero 1 ->[puntatore a i-node, nomefile]
        // readdir() -> dirent nomero 2 ->
        while ((dirp = readdir(dp)) != NULL) {
            debug("dir: %s\n", dirp->d_name);
            if ((strcmp(dirp->d_name, ".") == 0) || (strcmp(dirp->d_name, "..") == 0)) {  // dir ignora se stessa e genitore
                continue;                                                                 // va a prox ciclo
            }
            int size;
            char file_path[PATH_MAX];
            snprintf(file_path, sizeof(file_path), "%s/%s", path, dirp->d_name);

            if ((size = get_size(file_path)) >= 0) {  // recursion
                total_size += size;
            }
        }
        closedir(dp);
        debug("-");
        return total_size;
    }
	else{
		printf("ERROR type of file not allowed");
        exit(EXIT_FAILURE);
	}
}

///---------------------------------------------------

int fread_from_fwrite_to(FILE *s_input, FILE *s_output, int size_to_write, char *progress_bar_message) {
    debug("+read_from_write_to()\n");
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
        debug("1while()_total_read:%d\r", total_read);
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
    debug("-\n");
    // return filesize;
    return total_read;
}

//////////////////////---------------------FUNZIONI OPZIONALI------------------------/////////////////////////

void progress_bar(int processed, int total, char *message) {
    int progress = (int)((processed / (float)total) * 100);
    printf("%s progress: [%d%%] ->", message, progress);  //  %d per intero  %% per scrivere "%"  -> "message progress:[n%]
    printf(" processed= %ld\r", processed);
    fflush(stdout);
}

///---------------------------------------------------

int extimate_archive_size(int dir_size) {
    // ricavata a tentativi
    // spesso è corretta, a volte no...
    // su file da una certa dimensione in poi ~ 1MB l'errore è trascurabile
    debug("1extimate_archive_size()\n", 5);
    int expected_size = dir_size - (dir_size % 10240) + 10240;
    if ((dir_size % 10240) <= 8192)
        return expected_size;
    else
        return expected_size + 10240;
}

//////////////////////---------------------DEBUG------------------------/////////////////////////

// SUGGESTED USE: open new terminal window and run "tty" es. result->"dev/pts/1"
// copy result and in main window run "./server -d=10 2>dev/pts/1"
// this set debugLevel to 10 and redirect stderr to newly opened terminal
#ifdef DEBUG
	#include <stdarg.h>
	int debugLevel = 0;
		// default value 0 disable print
		// set different value via argument-> -d=value or -v=value or -d or -v for 10
		// note that max debug level in server is around 7
	int debugIndent = 0;
		// dont touch debug function uses it
#endif

///---------------------------------------------------

void parse_arg_for_debug_option(int argc, char *argv[]) {
#ifdef DEBUG
    while ((--argc) > 0) {
        int lvl = 10;
        char str[10] = "";
        sscanf(argv[argc], "%[^=]=%d", str, &lvl);
        if ((strcmp(str, "-d") == 0) || (strcmp(str, "-v") == 0)) {
            debugLevel = lvl;
			printf("SET DEBUG LEVEL %d\n", debugLevel);
            break;
        }
    }  // if not found 0
	printf("SET DEBUG LEVEL %d\n", debugLevel);
    return;
#endif
}

///---------------------------------------------------

int debug(const char *format, ...) {
#ifdef DEBUG
    va_list args;
    va_start(args, format);
    int indent = 0;

    if (*format != 0)
        switch (*format) {
        case '+':
            indent = (++debugIndent);
            break;
        case '-':
            indent = debugIndent > 0 ? debugIndent-- : 0;
            break;
        case '1':
            indent = debugIndent + 1;
            break;
        case '0':
            break;
        default:
            indent = debugIndent;
            format--;
            break;
        }
    format++;
    int count = 0;
    if (debugLevel > indent) {
        if (*format != '\0')
            while ((indent--) > 0) fprintf(stderr, "\t");
        fprintf(stderr, "\033[1;33m");

        while (*format != '\0') {
            if (*format != '%') {
                fputc(*format, stderr);
                count++;
            }
            else {
                // Handle format specifiers
                switch (*++format) {
                // funzionalità base printf
                case 'd':
                    count += fprintf(stderr, "%d", va_arg(args, int));
                    break;
                case 'c':
                    count += fputc(va_arg(args, int), stderr);
                    break;
                case 's':
                    count += fprintf(stderr, "%s", va_arg(args, char *));
                    break;
                case 'f':
                    count += fprintf(stderr, "%f", va_arg(args, double));
                    break;
                case '%':
                    fputc('%', stderr);
                    count++;
                    break;
                default:
                    // Handle unknown format specifiers
                    fputc('%', stderr);
                    fputc(*format, stderr);
                    count += 2;
                    break;
                }
            }
            format++;
        }
        fprintf(stderr, "\033[1;0m");
    }
    va_end(args);
    return count;
#endif
}

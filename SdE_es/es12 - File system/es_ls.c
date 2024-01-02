/*
 * es_ls.c
 * 
 * Questo programma mostra il contenuto della directory corrente. 
 * Per ogni entry, visualizza il numero dell'i-node, il nome del file e la sua
 * dimensione in byte.
 */

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h> 

int main(int argc, char* argv[]) 
{
    // --- GESTIONE PARAMETRI --- //
    // se non viene passato alcun parametro, considera la directory corrente
    char* dir_path;
    if (argc == 1)
    {
		dir_path = getcwd(NULL, 0);
		if (dir_path == NULL) 
		{
		    fprintf(stderr, "Impossibile ottenere la directory corrente\n");
		    exit(EXIT_FAILURE);
		}
    }
    else
    {
        dir_path = (char*)malloc(strlen(argv[1]));
        strcpy(dir_path, argv[1]);
    }
    
    // strutture per lo scorrimento delle entry della directory
    DIR *dp;
    struct dirent *dirp;
    
    // struttura per i metadati dei file
    struct stat file_stat;
    
    // spostamento nella directory specificata
    if ( chdir(dir_path) < 0)
    {
        fprintf(stderr, "Impossibile aprire '%s'\n", dir_path);
        exit(EXIT_FAILURE);
    }
         
    // apertura della directory corrente
    if ( (dp = opendir(".")) == NULL ) 
    {
        fprintf(stderr, "Impossibile aprire '%s'\n", dir_path);
        exit(EXIT_FAILURE);
    }

    printf("--- Contenuto della directory %s ---\n\n", dir_path);
    
    // scorre il contenuto della directory 
    while ( (dirp = readdir(dp)) != NULL )
    {
        // ottieni dimensione del file
        off_t file_size;
        if ( stat(dirp->d_name, &file_stat) < 0)
        {
            fprintf(stderr, "Impossibile recuperare metadati del file %s\n", dirp->d_name);
            exit(EXIT_FAILURE);
        }
        file_size = file_stat.st_size;
        
        // stampa numero di i-node, nome del file e sua dimensione
        printf("i-node:%ld\t file name: %s\t file size: %d\n", dirp->d_ino, dirp->d_name, file_size);
    }
    
    // chiusura della directory
    closedir(dp);
    
    return EXIT_SUCCESS;
}

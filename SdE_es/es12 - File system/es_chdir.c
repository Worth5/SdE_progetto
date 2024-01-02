/*
 * es_chdir.c
 *
 * Questo programma crea una nuova directory con il nome specificato
 * come parametro, e si sposta al suo interno.
 * Quando l'utente preme un tasto, il programma si sposta nuovamente
 * nella directory di partenza.
 *
 * Tra un'operazione e l'altra, aprire un'altra finestra del terminale
 * e vedere cosa cambia nell'output di "ls -l"
 * 
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Numero di argomenti errato\n");
        exit(EXIT_FAILURE);
    }

    // salvataggio della directory corrente
    char* orig_dir = getcwd(NULL,0);
    printf("La directory corrente e': %s\n", orig_dir);
    
    // creazione della nuova directory
    char* new_dir = (char*)malloc(strlen(argv[1]));
    strcpy(new_dir, argv[1]);
    
    mode_t perm = S_IRUSR | S_IWUSR | S_IXUSR;
    if ( mkdir(new_dir, perm) < 0)
    {
        fprintf(stderr, "Errore nella creazione della directory %s\n", new_dir);
        exit(EXIT_FAILURE);
    }
    printf("Directory %s creata correttamente\n", new_dir);
    
    // directory creata, spostamento al suo interno
    if ( chdir(new_dir) < 0)
    {
        fprintf(stderr, "Impossibile spostarsi nella directory %s\n", new_dir);
        exit(EXIT_FAILURE);
    }
    printf("Spostamento effettuato correttamente\n");
    
    char* current_dir = getcwd(NULL,0);
    printf("La directory corrente e': %s\n", current_dir);
    free(current_dir);
       
       
    // rimani in pausa fino a che l'utente non preme un tasto
    getchar();
    
    // ritorno alla directory originaria
    if ( chdir(orig_dir) < 0)
    {
        fprintf(stderr, "Impossibile spostarsi nella directory %s\n", orig_dir);
        exit(EXIT_FAILURE);
    }
    printf("Spostamento effettuato correttamente\n");
    
    current_dir = getcwd(NULL,0);
    printf("La directory corrente e': %s\n", current_dir);
    free(current_dir);
    
    
    
    free(orig_dir);
    free(new_dir);
    return EXIT_SUCCESS;
}

/* 
 * es5-1.c
 *
 */

#include <fcntl.h>   // for open() 
#include <stdio.h>   // for printf() and stderr
#include <stdlib.h>  // for exit()
#include <errno.h>   // for errno
#include <string.h>  // for strerror()
#include <unistd.h>  // for close()

// this function creates a file with the given name
void create_file(const char name[])
{
    int fd;
    char c;
    
    printf("File %s does not exist, do you want to create it? [y/N]\n",name);
    scanf("%c", &c);
    if (c == 'y')
    {
        // open file with O_CREAT flag
        // we also need to assign permissions
        mode_t perm = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH;
        fd = open(name, O_RDONLY|O_CREAT, perm);
        if (fd == -1)
        {
            fprintf(stderr, "error %d: %s \n", errno, strerror(errno));
            exit(EXIT_FAILURE);
        }
        printf("File created correctly\n");
        close(fd);
    }
}

// this function clears the content of the file with the given name
void trunc_file(const char name[])
{
    int fd;
    char c;
    
    printf("File %s exists, do you want to clear its content? [y/N]\n", name);
    scanf("%c", &c);
    if (c == 'y')
    {
        // open file with O_TRUNC flag
        fd = open(name, O_RDONLY|O_TRUNC);
        if (fd == -1)
        {
            fprintf(stderr, "errore %d: %s \n", errno, strerror(errno));
            exit(EXIT_FAILURE);
        }
        printf("File content cleared correctly\n");
        close(fd);
    }
}

int main()
{
    int fd;
    char name[] = "test";
    
    // try to open the file. If error, then the file does not exist
    if ( (fd = open(name, O_RDONLY)) == -1)
    {
        // file does not exist
        create_file(name);
    }
    else
    {
        // file exists
        close(fd);   
        trunc_file(name);
    }
    return 0;
}

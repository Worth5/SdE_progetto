/*
 * es_formatted_output.c
 */ 

#include <stdio.h>
#include <stdlib.h>


void scrivi_a_video(const char nome[], const char cognome[], int dd, int mm, int aaaa);
void scrivi_su_file(const char nome[], const char cognome[], int dd, int mm, int aaaa);
void scrivi_su_stringa(const char nome[], const char cognome[], int dd, int mm, int aaaa);

int main() 
{
    const char nome[] = "galileo";
    const char cognome[] = "galilei";
    int dd = 15;
    int mm = 2;
    int aaaa = 1564;
    
    scrivi_a_video(nome, cognome, dd, mm, aaaa);
    printf("==================\n");
    scrivi_su_file(nome, cognome, dd, mm, aaaa);
    printf("==================\n");
    scrivi_su_stringa(nome, cognome, dd, mm, aaaa);
    
    return EXIT_SUCCESS;
}


void scrivi_a_video(const char nome[], const char cognome[], int dd, int mm, int aaaa)
{
    printf("%s %s, nato il %02d/%02d/%d \n", nome, cognome, dd, mm, aaaa);
}

void scrivi_su_file(const char nome[], const char cognome[], int dd, int mm, int aaaa)
{
    fprintf(stdout, "%s %s, nato il %02d/%02d/%d \n", nome, cognome, dd, mm, aaaa);
}
void scrivi_su_stringa(const char nome[], const char cognome[], int dd, int mm, int aaaa)
{
    char str[100];
    sprintf(str, "%s %s, nato il %02d/%02d/%d \n", nome, cognome, dd, mm, aaaa);
    
    printf(str);
}



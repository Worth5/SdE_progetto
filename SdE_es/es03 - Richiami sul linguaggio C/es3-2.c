#include <stdio.h>
#include <string.h>

void concatena(char* strA, char* strB, char* str_dest)
{
    int i = 0;
    for (int j = 0; strA[j] != '\0'; j++)
    {
        str_dest[i] = strA[j];
        i++;
    }
    str_dest[i] = ' ';
    i++;   
    for (int j = 0; strB[j] != '\0'; j++)
    {
        str_dest[i] = strB[j];
        i++;
    }
    str_dest[i] = '\0';
}

int conta_carattere(char* str, char ch)
{
    int contatore = 0;
    for (int i = 0; i<strlen(str); i++)
    {
        if (str[i] == ch)
            contatore++;
    }
    return contatore;
}
    


int main()
{
    const int MAXLEN = 100;
    char s1[MAXLEN+1];
    char s2[MAXLEN+1];
    
    struct Coppia {
        char min[MAXLEN+1];
        char max[MAXLEN+1];
    } coppia;
    
    printf("Inserisci la prima parola: \n");
    scanf("%s", s1);
    printf("Inserisci la seconda parola: \n");
    scanf("%s", s2);
    
    // mi assicuro che le parole inserite non superino i 100 caratteri
    s1[MAXLEN] = '\0';
    s2[MAXLEN] = '\0';
    
    // individua stringa minore e maggiore
    if (strcmp(s1, s2) < 0)
    {
        strcpy(coppia.min, s1);
        strcpy(coppia.max, s2);
    }
    else
    {
        strcpy(coppia.min, s2);
        strcpy(coppia.max, s1);
    }
    
    // concatena le stringhe
    char str[2*MAXLEN+1+1];  // si riserva spazio per i caratteri ' ' e '\0' 
    concatena(coppia.min, coppia.max, str);  
    printf("\nLa stringa concatenata e' %s\n", str);

        
    // conta le occorrenze del carattere letto
    char ch = 'a';
    int num_ch = conta_carattere(str, ch);
    printf("\nIl carattere '%c' compare %d volte\n", ch, num_ch);
    
    return 0;
}

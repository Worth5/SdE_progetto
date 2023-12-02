#include <stdio.h>

int main()
{
    int a,b;
    
    printf("Leggi a:\n");
    scanf("%d", &a);
    
    printf("Leggi b:\n");
    scanf("%d",&b);
    
    int somma = a+b;
    printf("La somma di a+b e': %d\n", somma);
    
    return 0;
}

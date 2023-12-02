#include <stdio.h>

int trova_max(int *v, int dim)
{
    int max = v[0];
    for (int i=1; i<dim; i++)
    {
        if (v[i] > max)
            max = v[i];
    }
    return max;
}

int main()
{
    const int TOT = 5;
    int vett[TOT];
    int n, cont=0;
    printf("Inserisci 5 numeri interi positivi:\n");
    
    while (cont < TOT)
    {
        scanf("%d", &n);
        if (n >= 0)
        {
            vett[cont] = n;
            cont++;
        }    
    }
    
    // stampa array
    printf("Valori inseriti nell'array: "); 
    for (int i=TOT-1; i>=0; i--)
    {
        printf("%d ", vett[i]);
    }
    printf("\n");
    
    // stampa valore massimo
    int max = trova_max(vett,TOT); 
    printf("Il massimo e': %d\n", max);
    
    return 0;
}

#include <stdio.h>
#include <arpa/inet.h>

void print_bytes(uint32_t num)
{
    char* c;
    c = (char*)&num;
    
    // stampa i 4 byte del numero, a partire dall'indirizzo piu' alto
    printf("%ld -> %d \n", &c[3], c[3]);
    printf("%ld -> %d \n", &c[2], c[2]);
    printf("%ld -> %d \n", &c[1], c[1]);
    printf("%ld -> %d \n", &c[0], c[0]);
}

int main()
{
    uint32_t host_num = 1;
 
    printf("Host number: %d \n", host_num);
    print_bytes(host_num);

/*    
    // converti in network byte order
    uint32_t net_num = htonl(host_num);
    
    printf("\nNetwork number: %d\n", net_num);
    print_bytes(net_num);
*/    
    return 0;
}

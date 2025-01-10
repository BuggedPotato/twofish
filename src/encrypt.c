#include<stdio.h>
#include "../include/twofish.h"
#include "../include/utils.h"

int main(int argc, char *argv[]){

    printf("cheeki breeki\n");
    DWORD a, b;
    a = 0xFFFFFFFA;
    b = 0xF;
    PHT(&a, &b);
    printf("%u %u\n", a, b);

    DWORD tab[8];
    parseHex( 192, tab, "ABC123DEF456FEd789cBaABC123DEF456FEd789cBa11" );
    for(int i = 0; i < 8; i++)
        printf("%X\n", tab[i]);

    return 0;
}
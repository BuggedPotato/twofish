#include<stdio.h>
#include "../include/twofish.h"

int main(int argc, char *argv[]){

    printf("cheeki breeki\n");
    DWORD a, b;
    a = 0xFFFFFFFA;
    b = 0xF;
    PHT(&a, &b);
    printf("%u %u\n", a, b);

    return 0;
}
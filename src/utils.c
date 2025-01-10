#include <stdio.h>
#include <errno.h>
#include "../include/types.h"
/*
circular bit shift
*/
BYTE ROR4(BYTE val, int shift){
    BYTE a = val >> shift;
    BYTE b = val << (sizeof(val) * 8 - shift);
    return a | b;
}

BYTE ROL4(BYTE val, int shift){
    BYTE a = val << shift;
    BYTE b = val >> (sizeof(val) * 8 - shift);
    return a | b;
}

DWORD ROR32(DWORD val, int shift){
    DWORD a = val >> shift;
    DWORD b = val << (sizeof(val) * 8 - shift);
    return a | b;
}

DWORD ROL32(DWORD val, int shift){
    DWORD a = val << shift;
    DWORD b = val >> (sizeof(val) * 8 - shift);
    return a | b;
}

int parseHex( int size, DWORD *dest, char *text ){
    if( !dest )
        perror("NULL dest while parsing");

    for( int i = 0; i * 8 < size; i++ )
        dest[i] = 0;
        
    char c;
    DWORD hexVal;
    for( int i = 0; i * 4 < size; i++ ){
        c = text[i];
        if ( c >= '0' && c <= '9' )
			hexVal = c - '0';
		else if ( c >= 'a' && c <= 'f' )
			hexVal = c - 'a' + 10;
		else if ( c >= 'A' && c <= 'F' )
			hexVal = c - 'A' + 10;
        else
            return -1;

        printf("shift: %d, hexVal: %X\n", (7 - i % 8), hexVal);
        printf( "hexVal: %X\n", hexVal << (4 * (7 - i % 8)) );
        dest[i/8] |= hexVal << (4 * (7 - i % 8));
    }

    return 0;
}
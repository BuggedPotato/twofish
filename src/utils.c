#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "../include/types.h"
/*
circular bit shift
*/
BYTE ROR4(BYTE val, int shift){
    BYTE a = val >> shift % 4;
    BYTE b = val << (sizeof(val) * 8 - shift) % 4;
    return (a | b) & 0x0F;
}

BYTE ROL4(BYTE val, int shift){
    BYTE a = val << shift % 4;
    BYTE b = val >> (sizeof(val) * 8 - shift) % 4;
    return (a | b) & 0x0F;;
}

DWORD ROR32(DWORD val, int shift){
    DWORD a = val >> shift % 32;
    DWORD b = val << (sizeof(val) * 8 - shift) % 32;
    return a | b;
}

DWORD ROL32(DWORD val, int shift){
    DWORD a = val << shift % 32;
    DWORD b = val >> (sizeof(val) * 8 - shift) % 32;
    return a | b;
}

// size in bits
int parseHex( int size, DWORD *dest, char *text ){
    if( !dest ){
        perror("NULL dest while parsing");
        return -1;
    }
    for( int i = 0; i * 32 < size; i++ )
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
            return 1;
        dest[i/8] |= hexVal << (4 * ((i ^ 1) % 8) );
    }
    return 0;
}

U_DWORD reverseBytes( U_DWORD x ){
    int j = 3;
    int k = 0;
    BYTE tmp = x.bytes[k];
    for( k = 0; k < j; k++ ){
        x.bytes[k] = x.bytes[j];
        x.bytes[j] = tmp;
        j--;
    }
    return x;
}

void safeExit(int code, FILE *input, FILE *output){
    if( input ) fclose( input );
    if( output ) fclose( output );
    exit(code);
}

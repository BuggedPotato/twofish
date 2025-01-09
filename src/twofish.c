#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include "../include/types.h"
#include "../include/utils.h"
#include "../include/tables.h"

BYTE q( int perm, BYTE x );
DWORD h(U_DWORD wordX, U_DWORD * listL, int k);
void PHT( DWORD *a, DWORD *b );

void f( DWORD r0, DWORD r1, DWORD *f0, DWORD *f1 ){
    r1 <<= 8;
    
}

/*
    Function h and g
    h generates keys to add to g output
*/
DWORD h(U_DWORD wordX, U_DWORD * listL, int k){
    if( !listL ){
        errno = ENOMEM;
        perror("NULL pointer L list");
    }
    BYTE y[4];
    // BYTE l[4][4];
    BYTE x[4];
    /*
    for( int i = 0; i < k; i ++ ){
        for( int j = 0; j < 4; j++ ){
            // split listL into bytes
            l[i][j] = listL[i].bytes[j];
            // l[i][j] = ( (int) listL[i] / (1 << (8 * j)) ) % (1 << 8);
            // x[j] = int(wordX / (1 << (8 * j))) % (1 << 8);
        }
    }
    */

    // y is reverse bytes of x
    {
        int i = 3;
        for( int j = 0; j < 4 && i >= 0; j++ ){
            y[j] = x[i];
            i--;
        }
    }

    switch( k ){
        case 4:
            y[0] = q(1, y[0]) ^ listL[3].bytes[0];
            y[1] = q(0, y[1]) ^ listL[3].bytes[1];
            y[2] = q(0, y[2]) ^ listL[3].bytes[2];
            y[3] = q(1, y[3]) ^ listL[3].bytes[3];
            // fallthrough on purpose
        case 3:
            y[0] = q(1, y[0]) ^ listL[2].bytes[0];
            y[1] = q(1, y[1]) ^ listL[2].bytes[1];
            y[2] = q(0, y[2]) ^ listL[2].bytes[2];
            y[3] = q(0, y[3]) ^ listL[2].bytes[3];
            // fallthrough on purpose
        default:
            y[0] = q( 1, q( 0, q(0, y[0]) ^ listL[1].bytes[0] ) ^ listL[0].bytes[0] );
            y[1] = q( 0, q( 0, q(1, y[1]) ^ listL[1].bytes[1] ) ^ listL[0].bytes[1] );
            y[2] = q( 1, q( 1, q(0, y[2]) ^ listL[1].bytes[2] ) ^ listL[0].bytes[2] );
            y[3] = q( 0, q( 1, q(1, y[3]) ^ listL[1].bytes[3] ) ^ listL[0].bytes[3] );
    }

    // MDS multiplication
    U_DWORD z;
    for( int i = 0; i < 4; i++ ){
        for( int j = 0; j < 4; j++ ){
            z.bytes[i] ^= MDS[i][j] * y[j];
        }
    }
    return z.dword;
}

BYTE q( int perm, BYTE x ){
    BYTE a0, b0;
    a0 = x >> 4;
    b0 = x % 16;
    BYTE a1 = a0 ^ b0;
    BYTE b1 = a0 ^ ROR4(b0, 1) ^ (8*a0 % 16);

    BYTE a2 = T_TABLES[perm][0][a1];
    BYTE b2 = T_TABLES[perm][1][b1];

    BYTE a3 = a2 ^ b2;
    BYTE b3 = a2 ^ ROR4(b2, 1) ^ (8*a2 % 16);

    BYTE a4 = T_TABLES[perm][2][a3];
    BYTE b4 = T_TABLES[perm][3][b3];

    return (b4 << 4) + a4;
}

// Pseudo-Hadamard Transform
void PHT( DWORD *a, DWORD *b ){
    if( !a || !b ){
        errno = ENOMEM;
        perror( "NULL pointer in PHT" );
    }
    DWORD aCopy = *a;
    DWORD bCopy = *b;
    printf("%u %u\n", aCopy, bCopy);
    // modulo 2^32 taken care of by type limits
    *a = (aCopy + bCopy);
    *b = (aCopy + 2*bCopy);
}
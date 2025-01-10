#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include "../include/types.h"
#include "../include/utils.h"
#include "../include/tables.h"
#include "../include/enums.h"
#include "../include/constants.h"

BYTE q( int perm, BYTE x );
U_DWORD h(U_DWORD wordX, U_DWORD * listL, int k);
void PHT( DWORD *a, DWORD *b );

/*
    Function h and g
    h generates keys to add to g output
*/
static U_DWORD h(U_DWORD wordX, U_DWORD * listL, int keyLength){
    if( !listL ){
        errno = ENOMEM;
        perror("NULL pointer L list");
    }
    int k = (keyLength + 63) / 64;
    BYTE y[4];
    // BYTE l[4][4];
    BYTE x[4];
    for( int i = 0; i < k; i ++ )
        x[i] = wordX.bytes[i];
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
    return z;
}

static BYTE q( int perm, BYTE x ){
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


void keyInit( keyObject *key, direction direction, int keyLength, char *keyRaw ){
    if (key == NULL)			
		perror("NULL key init");
	if ( (direction != ENCRYPT) && (direction != DECRYPT) )
		perror("Invalid direction");
	if ( keyLength > MAX_KEY_SIZE || keyLength < 8 )	
		perror("Invalid key length");
	
    key->direction = direction;
    key->keyLength = (keyLength + 63) & ~63; // rounds up to multiple of 64
    if( parseHex( keyLength, key->keyDWords, keyRaw ) )
        perror("Invalid hexadecimal key material");
    
    generateRoundKeys(key);
}

static void generateRoundKeys( keyObject *key ){
    DWORD keyOdds[MAX_KEY_SIZE / 64];
    DWORD keyEvens[MAX_KEY_SIZE / 64];

    // int condition = (key->keyLength + 63) / 64;	// round up?
    int condition = key->keyLength;
    for( int i = 0; i < condition; i++ ){
        keyOdds[i] = key->keyDWords[2*i];
        keyEvens[i] = key->keyDWords[2*i+1];

        // for RS multiplication
        union {
            BYTE bytes[8];
            DWORD dwords[2];
        } tmpKey;
        tmpKey.dwords[0] = keyEvens[i];
        tmpKey.dwords[1] = keyOdds[i];

        // RS multiplication
        key->sBoxKeys[i].dword = 0;
        for( int j = 0; j < condition; j++ ){
            for( int k = 0; k < 8; k++ ){
                // reverse
                // TODO MAYBE ERROR index
               key->sBoxKeys[condition-i-1].bytes[condition-j-1] += tmpKey.bytes[k] * RS[j][k];
            }
        }
    }

    // generate round keys
    for( int i = 0; i < ROUND_KEYS/2; i++ ){
        U_DWORD roundKey0, roundKey1;
        U_DWORD i0, i1;
        i0.dword = i*RK_CONST; // 2*i 
        i1.dword = i*RK_CONST + RK_CONST_SHIFT; // 2*i+1
        roundKey0 = h( i0, keyEvens, key->keyLength );
        roundKey1 = h( i1, keyOdds, key->keyLength );
        roundKey1.dword = ROL32( roundKey1.dword, 8 );
        
        // PHT
        key->roundKeys[2*i] = roundKey0.dword + roundKey1.dword;
        key->roundKeys[2*i+1] = roundKey0.dword + 2*roundKey1.dword;
        key->roundKeys[2*i+1] = ROL32(key->roundKeys[2*i+1], 9);
    }
}

int initCipher( cipherObject *cipher, mode mode ){
    if( !cipher ){
        perror("NULL cipher object in intit");
    }
    // TODO IV

    cipher->mode = mode;
    return 0;
}

int encryptBlock( cipherObject *cipher, keyObject *key, BYTE *input, int inputLength, BYTE *output ){

    if ( inputLength % BLOCK_SIZE ) // invalid input length
            return 2;

    if( cipher->mode == CBC ){
        // TODO IV
    }

    U_DWORD x[BLOCK_SIZE/4];
    for( int block = 0; block < inputLength; block += BLOCK_SIZE ){
        for( int i = 0; i < BLOCK_SIZE/32; i++ ){
            x[i] = reverseBytes( ((U_DWORD *)input)[i] );
            // input whitening
            x[i].dword ^= key->roundKeys[i]; 
            // TODO CBC

            // main loop
            U_DWORD r0, r1;
            for( int round = 0; round < MAX_ROUNDS; round++ ){
                // actually g funtions
                r0 = h( x[0], key->sBoxKeys, key->keyLength );
                r1.dword = ROL32(x[1].dword, 8);
                r1 = h( r1, key->sBoxKeys, key->keyLength );

                PHT( &r0.dword, &r1.dword );
                r0.dword += key->roundKeys[2*round + 8];
                r1.dword += key->roundKeys[2*round + 9];
                x[3].dword = ROL32( x[3].dword, 1 );
                r0.dword ^= x[2].dword;
                r0.dword = ROR32( r0.dword, 1 );
                r1.dword ^= x[3].dword;

                // per round swap unless its last round
                if( round < MAX_ROUNDS - 1 ){
                    x[0] = r0;
                    x[1] = r1;
                    x[2] = x[0];
                    x[3] = x[1];
                }
            }
        }

        for( int i = 0; i < BLOCK_SIZE/32; i++ ){
            // output whitening
            x[i].dword ^= key->roundKeys[BLOCK_SIZE/32 + i];
            ((U_DWORD *)output)[i] = reverseBytes( x[i] );
            // TODO CBC
        }

        input += BLOCK_SIZE / 8;
        output += BLOCK_SIZE / 8;
    }

    return 0;
}


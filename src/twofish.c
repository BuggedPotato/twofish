#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include "../include/types.h"
#include "../include/utils.h"
#include "../include/tables.h"
#include "../include/enums.h"
#include "../include/constants.h"

static BYTE q( int perm, BYTE x );
void PHT( DWORD *a, DWORD *b );
static int generateRoundKeys( keyObject *key );

// multiplication in GF(256)
DWORD GF256Mult(BYTE a, BYTE b){
    DWORD p = 0;
    BYTE carry = 0;
    for( int i = 0; i < 8 && a!= 0 && b != 0; i++ ){
        if( b & 1 ) // rightmost bit
            p ^= a;
        b >>= 1;
        carry = a & 0b10000000; // leftmost bit
        a <<= 1;
        if( carry )
            a ^= GF_256_PRIM_POLY_NO_HIGH;
    }
    return p;
}

/*
    Function h and g
    h generates keys to add to g output
*/
U_DWORD h(U_DWORD wordX, U_DWORD *listL, int keyLength){
    if( !listL ){
        errno = ENOMEM;
        perror("NULL pointer L list");
        exit(1);
    }
    int k = (keyLength + 63) / 64;
    BYTE y[4];

    // y is reverse bytes of x
    for( int i = 0; i < 4; i++ ){
        y[i] = wordX.bytes[i];
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
    z.dword = 0;
    for( int i = 0; i < 4; i++ ){
        for( int j = 0; j < 4; j++ ){
            z.dword ^= GF256Mult( MDS[i][j], y[j] ) << 8*i;
        }
    }
    return z;
}

static BYTE q( int perm, BYTE x ){
    BYTE a0, b0;
    a0 = x >> 4;
    b0 = x % 16;
    BYTE a1 = a0 ^ b0;
    BYTE b1 = a0 ^ ROR4(b0, 1) ^ 8*a0 % 16;

    BYTE a2 = T_TABLES[perm][0][a1];
    BYTE b2 = T_TABLES[perm][1][b1];

    BYTE a3 = a2 ^ b2;
    BYTE b3 = a2 ^ ROR4(b2, 1) ^ 8*a2 % 16;

    BYTE a4 = T_TABLES[perm][2][a3];
    BYTE b4 = T_TABLES[perm][3][b3];

    return (b4 << 4) + a4;
}

// Pseudo-Hadamard Transform
void PHT( DWORD *a, DWORD *b ){
    if( !a || !b ){
        errno = ENOMEM;
        perror( "NULL pointer in PHT" );
        exit(errno);
    }
    DWORD aCopy = *a;
    DWORD bCopy = *b;
    *a = (aCopy + bCopy);
    *b = (aCopy + 2*bCopy);
}


static DWORD RSRemainder(DWORD x){
    BYTE  b  = (BYTE) (x >> 24); /* MSB */
    DWORD g2 = ((b << 1) ^ ((b & 0x80) ? 0x14D : 0 )) & 0xFF;
    DWORD g3 = ((b >> 1) & 0x7F) ^ ((b & 1) ? 0x14D >> 1 : 0 ) ^ g2 ;
    return ((x << 8) ^ (g3 << 24) ^ (g2 << 16) ^ (g3 << 8) ^ b);
}

/*
    Use (12,8) Reed-Solomon code over GF(256) to produce
    a key S-box dword from two key material dwords.
*/
static DWORD getSBoxKey(DWORD k0,DWORD k1)
{
    DWORD r = 0;
    for ( int i = 0; i < 2; i++ )
    {
        r ^= i ? k0 : k1;	
        for (int j = 0; j < 4; j++ )
            r = RSRemainder(r);				
    }
    return r;
}


int initKey( keyObject *key, direction direction, int keyLength, char *keyRaw ){
    if (key == NULL){
		perror("NULL key init");
        return 1;
    }		
	if ( (direction != ENCRYPT) && (direction != DECRYPT) ){
		perror("Invalid direction");
        return 2;
    }
	if ( keyLength > MAX_KEY_SIZE || keyLength < 8 ){
		perror("Invalid key length");
        return 3;
    }
	
    key->direction = direction;
    key->keyLength = (keyLength + 63) & ~63; // rounds up to multiple of 64
    if( parseHex( keyLength, key->keyDWords, keyRaw ) ){
        printf("%s %d\n", keyRaw, keyLength);
        perror("Invalid hexadecimal key material");
        return 4;
    }

    #if DEBUG
        printf("input key length: %d\n", keyLength);
        printf( "set key length: %d\n", key->keyLength );
        for( int i = 0; i < key->keyLength / 32; i++ )
            printf( "key dwords: %08X\n", key->keyDWords[i] );
        printf("\n");
    #endif

    return generateRoundKeys(key);
}

static int generateRoundKeys( keyObject *key ){
    DWORD keyOdds[MAX_KEY_SIZE / 64];
    DWORD keyEvens[MAX_KEY_SIZE / 64];

    int condition = key->keyLength / 64;
    for( int i = 0; i < condition; i++ ){
        keyEvens[i] = key->keyDWords[2*i];
        keyOdds[i] = key->keyDWords[2*i+1];

        key->sBoxKeys[condition-i-1].dword = getSBoxKey( keyEvens[i], keyOdds[i] );
    }
     #if DEBUG
            printf("key dwords even:\n");
            for( int j = 0; j < condition; j++ )
                printf( "%08X ", keyEvens[j] );
            printf("\nkey dwords odd:\n");
            for( int j = 0; j < condition; j++ )
                printf( "%08X ", keyOdds[j] );
            printf("\n");
            for( int j = 0; j < condition; j++ )
                printf( "sbox keys[%d]: %08X\n", j, key->sBoxKeys[j].dword );
            printf("round keys:\n");
        #endif

    // generate round keys
    for( int i = 0; i < ROUND_KEYS/2; i++ ){
        U_DWORD roundKey0, roundKey1;
        U_DWORD i0, i1;
        i0.dword = i*RK_CONST; // 2*i 
        i1.dword = i*RK_CONST + RK_CONST_SHIFT; // 2*i+1
        roundKey0 = h( i0, (U_DWORD *)keyEvens, key->keyLength );
        roundKey1 = h( i1, (U_DWORD *)keyOdds, key->keyLength );
        roundKey1.dword = ROL32( roundKey1.dword, 8 );
        
        // PHT
        key->roundKeys[2*i] = roundKey0.dword + roundKey1.dword;
        key->roundKeys[2*i+1] = roundKey0.dword + 2*roundKey1.dword;
        key->roundKeys[2*i+1] = ROL32(key->roundKeys[2*i+1], 9);
        #if DEBUG
            printf( "roundKeys[%d] = %08X\nroundKeys[%d] = %08X\n", 2*i, key->roundKeys[2*i], 2*i+1, key->roundKeys[2*i+1] );
        #endif
    }
    return 0;
}

int initCipher( cipherObject *cipher, mode mode, char *ivRaw, int ivLength ){
    if( !cipher ){
        perror("NULL cipher object in init");
        return 1;
    }
    if( mode == CBC ){
        if( ivRaw == NULL ){
            perror( "no initialisation vector for CBC mode" );
            return 2;
        }
        if ( ivLength != BLOCK_SIZE ){
		perror("Invalid initialisation vector length");
        return 3;
        }
        if( parseHex( ivLength, cipher->iv, ivRaw ) ){
            perror("Invalid initialisation vector material");
            return 4;
        }
    }
    cipher->mode = mode;
    return 0;
}

/*

*/
int encryptBlock( cipherObject *cipher, keyObject *key, int inputLength, BYTE *input, BYTE *output ){

    if ( inputLength % BLOCK_SIZE ) // invalid input length
            return 2;


    U_DWORD x[BLOCK_SIZE/32];
    for( int block = 0; block < inputLength; block += BLOCK_SIZE ){
        #if DEBUG
            printf("BLOCK: %d\n", block);
        #endif
        for( int i = 0; i < BLOCK_SIZE/32; i++ ){
            x[i] = ((U_DWORD *)input)[i];
            if( cipher->mode == CBC ){
                x[i].dword ^= cipher->iv[i];
            }
            // input whitening
            x[i].dword ^= key->roundKeys[i]; 
            #if DEBUG
                printf( "x[%d] = %08X\n", i, x[i].dword );
            #endif
        }

        // main loop
        U_DWORD r0, r1;
        for( int round = 0; round < MAX_ROUNDS; round++ ){
            // actually g funtions
            r0 = h( x[0], key->sBoxKeys, key->keyLength );
            r1.dword = ROL32(x[1].dword, 8);
            r1 = h( r1, key->sBoxKeys, key->keyLength );

            #if DEBUG
                printf( "r0: %08X, r1: %08X\n", r0.dword, r1.dword );
            #endif

            PHT( &r0.dword, &r1.dword );
            r0.dword += key->roundKeys[2*round + BLOCK_SIZE/16];
            r1.dword += key->roundKeys[2*round + BLOCK_SIZE/16 + 1];
            
            x[3].dword = ROL32( x[3].dword, 1 );
            x[2].dword ^= r0.dword;
            x[3].dword ^= r1.dword;
            x[2].dword = ROR32( x[2].dword, 1 );

            // per round swap unless its last round
            if( round < MAX_ROUNDS - 1 ){
                U_DWORD tmp = x[0];
                x[0] = x[2];
                x[2] = tmp;
                tmp = x[1];
                x[1] = x[3];
                x[3] = tmp;
            }
        }
        
        for( int i = 0; i < BLOCK_SIZE/32; i++ ){
            // output whitening
            x[i].dword ^= key->roundKeys[BLOCK_SIZE/32 + i];
            ((U_DWORD *)output)[i] = x[i];
            if( cipher->mode == CBC ){
                cipher->iv[i] = (x[i].dword);
            }
        }

        input += BLOCK_SIZE / 8;
        output += BLOCK_SIZE / 8;
    }
    return 0;
}

int decryptBlock( cipherObject *cipher, keyObject *key, int inputLength, BYTE *input, BYTE *output ){

    if ( inputLength % BLOCK_SIZE ) // invalid input length
            return 2;

    U_DWORD x[BLOCK_SIZE/32];
    for( int block = 0; block < inputLength; block += BLOCK_SIZE ){
        #if DEBUG
            printf("BLOCK: %d\n", block);
        #endif
        for( int i = 0; i < BLOCK_SIZE/32; i++ ){
            x[i] = ((U_DWORD *)input)[i];
            // input whitening second half of keys used
            x[i].dword ^= key->roundKeys[BLOCK_SIZE/32 + i]; 
            #if DEBUG
                printf( "x[%d] = %08X\n", i, x[i].dword );
            #endif
            // TODO CBC
        }

        // main loop
        U_DWORD r0, r1;
        for( int round = MAX_ROUNDS - 1; round >= 0; round-- ){
            // actually g funtions
            r0 = h( x[0], key->sBoxKeys, key->keyLength );
            r1.dword = ROL32(x[1].dword, 8);
            r1 = h( r1, key->sBoxKeys, key->keyLength );

            #if DEBUG
                printf( "r0: %08X, r1: %08X\n", r0.dword, r1.dword );
            #endif

            PHT( &r0.dword, &r1.dword );
            r0.dword += key->roundKeys[2*round + BLOCK_SIZE/16];
            r1.dword += key->roundKeys[2*round + BLOCK_SIZE/16 + 1];
            
            // reverse order and direction of rotates
            x[2].dword = ROL32( x[2].dword, 1 );
            x[2].dword ^= r0.dword;
            x[3].dword ^= r1.dword;
            x[3].dword = ROR32( x[3].dword, 1 );

            // per round swap unless its last round
            if( round > 0 ){
                U_DWORD tmp = x[0];
                x[0] = x[2];
                x[2] = tmp;
                tmp = x[1];
                x[1] = x[3];
                x[3] = tmp;
            }
        }
        
        for( int i = 0; i < BLOCK_SIZE/32; i++ ){
            // output whitening
            x[i].dword ^= key->roundKeys[i];
            if( cipher->mode == CBC ){
                x[i].dword ^= cipher->iv[i];
                cipher->iv[i] = ((DWORD *)input)[i];
            }
            ((U_DWORD *)output)[i] = x[i];
            // TODO CBC
        }

        input += BLOCK_SIZE / 8;
        output += BLOCK_SIZE / 8;
    }
    return 0;
}

#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include "../include/twofish.h"
#include "../include/utils.h"
#include "../include/types.h"

#define BUFFER_SIZE 128

int main(int argc, char *argv[]){

    FILE *inputFile = fopen( "nightcall_ECB_cd0fd166775dc3f368aa41840482202c.bin", "rb" );
    if( inputFile == NULL ){
        errno = ENOENT;
        perror( "Could not open input file" );
        exit(errno);
    }
    FILE *outputFile = fopen( "output.bin", "wb" );
    if( outputFile == NULL ){
        fclose( inputFile );
        errno = ENOENT;
        perror( "Could not open output file" );
        exit(errno);
    }

    BYTE inputBuffer[BUFFER_SIZE], outputBuffer[BUFFER_SIZE];
    memset( inputBuffer, 0, BUFFER_SIZE );
    memset( outputBuffer, 0, BUFFER_SIZE );

    char keyRaw[MAX_KEY_ASCII_SIZE] = "cd0fd166775dc3f368aa41840482202c";
    int keyLength = strlen(keyRaw) * 4; // bit length
    direction direction = DECRYPT;
    keyObject key;
    cipherObject cipher;
    initKey( &key, direction, keyLength, keyRaw );
    initCipher( &cipher, ECB );
    int (*cipherFunction)(cipherObject *cipher, keyObject *key, int inputLength, BYTE *input, BYTE *output);

    switch (direction)
    {
        case ENCRYPT:
            cipherFunction = encryptBlock;
            break;
        case DECRYPT:
            cipherFunction = decryptBlock;
        break;
        default:
            perror("Invalid direction");
            break;
    }

    int readBytes = 0;
    while( (readBytes = fread( inputBuffer, sizeof(BYTE), BUFFER_SIZE/8, inputFile )) != 0 ){
        if( cipherFunction( &cipher, &key, BUFFER_SIZE, inputBuffer, outputBuffer ) ){
            perror("failed! invalid input size");
            fclose( inputFile );
            fclose( outputFile );
            exit(2);
        }

        #if DEBUG
            printf( "read bytes: %d\n", readBytes );
            printf( "read input:\n" );
            for( int i = 0; i < BUFFER_SIZE/8; i++ ){
                printf( "%02X ", inputBuffer[i] );
                if( (i+1) % 16 == 0 )
                    printf("\n");
            }
            printf("\n");
            printf( "written output:\n" );
            for( int i = 0; i < BUFFER_SIZE/8; i++ ){
                printf( "%02X ", outputBuffer[i] );
                if( (i+1) % 16 == 0 )
                    printf("\n");
            }
            printf("\n");
        #endif
        fwrite( outputBuffer, sizeof(BYTE), BUFFER_SIZE/8, outputFile );
        // fflush( outputFile );
        memset( inputBuffer, 0, BUFFER_SIZE/8 );
    }

    fclose( inputFile );
    fclose( outputFile );
    
    return 0;
}
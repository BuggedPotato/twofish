#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include "../include/twofish.h"
#include "../include/utils.h"
#include "../include/types.h"

#define BUFFER_SIZE 128

int main(int argc, char *argv[]){

    FILE *inputFile = fopen( "input.txt", "rb" );
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

    char keyRaw[MAX_KEY_ASCII_SIZE] = "6cec81cdb8e816dc3a5d97475fc52937";
    int keyLength = strlen(keyRaw) * 4; // bit length
    keyObject key;
    cipherObject cipher;
    initKey( &key, ENCRYPT, keyLength, keyRaw );
    initCipher( &cipher, ENCRYPT );

    int readBytes = 0;
    while( (readBytes = fread( inputBuffer, sizeof(BYTE), BUFFER_SIZE/8, inputFile )) != 0 ){
        if( encryptBlock( &cipher, &key, BUFFER_SIZE, inputBuffer, outputBuffer ) ){
            perror("encryption failed invalid input size");
            fclose( inputFile );
            fclose( outputFile );
            exit(2);
        }

        #if DEBUG
            printf( "read bytes: %d\n", readBytes );
            // printf( "read input:\n" );
            // for( int i = 0; i < BUFFER_SIZE/8; i++ ){
            //     printf( "%02X ", inputBuffer[i] );
            //     if( (i+1) % 16 == 0 )
            //         printf("\n");
            // }
            // printf("\n");
            // printf( "written output:\n" );
            // for( int i = 0; i < BUFFER_SIZE/8; i++ ){
            //     printf( "%02X ", outputBuffer[i] );
            //     if( (i+1) % 16 == 0 )
            //         printf("\n");
            // }
            // printf("\n");
        #endif
        fwrite( outputBuffer, sizeof(BYTE), BUFFER_SIZE/8, outputFile );
        // fflush( outputFile );
        memset( inputBuffer, 0, BUFFER_SIZE/8 );
    }

    fclose( inputFile );
    fclose( outputFile );
    
    return 0;
}
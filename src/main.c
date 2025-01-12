#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<ctype.h>
#include<string.h>
#include "../include/twofish.h"
#include "../include/utils.h"
#include "../include/types.h"

#define BUFFER_SIZE (BLOCK_SIZE / 8 * 4)

int main(int argc, char *argv[]){
    /*
        arguments validation
    */
    char *inputFileName = argv[1];
    char *outputFileName = argv[2];
    char *arg;

    char keyRaw[MAX_KEY_ASCII_SIZE+1];
    char ivRaw[BLOCK_SIZE/4+1];
    direction direction = ENCRYPT;
    mode mode = ECB;

    if ( argc > 1 && strcmp(argv[1], "-h") == 0 ){
        printf("Usage: main[.exe] input_file output_file ARGUMENTS\n" \
        "Arguments:\n\t-m [ebc|cbc]\n\t-d [e|d]\n\t-k [key]\n\t-v [vector]\n" \
        "-m - algorithm mode of operation:\n\tEBC\n\tCBC (requires the -v argument)\n-d - direction:\n\te = encrypt\n\td = decrypt\n" \
        "-k - key for the algorithm made from 8-64 hexadecimal characters\n" \
        "-v - initilisation vector used with CBC mode, %d hexadecimal characters\n", BLOCK_SIZE/4);
        exit(0);
    }
    if( argc < 9 ){
        perror("Not enough arguments");
        exit(-1);
    }


    int keyLength, ivLength;

    for( int i = 3; i < argc-1; i += 2 ){
        arg = argv[i];
        if( arg[0] != '-' ){
            perror("Invalid arguments");
            exit(2);
        }
        else if ( strcmp(arg, "-d") == 0 ){
            if( strcmp( argv[i+1], "e" ) == 0 )
                direction = ENCRYPT;
            else if( strcmp( argv[i+1], "d" ) == 0 )
                direction = DECRYPT;
            else{
                perror("Invalid direction argument value");
                exit(1);
            }
        }
        else if( strcmp(arg, "-m") == 0 ){
            if( strcmp( argv[i+1], "ecb" ) == 0 )
                mode = ECB;
            else if( strcmp( argv[i+1], "cbc" ) == 0 )
                mode = CBC;
            else{
                perror("Invalid direction argument value");
                exit(1);
            }
        }
        else if( strcmp(arg, "-v") == 0 ){
            int len = strlen(argv[i+1]) + 1;
            if( len > BLOCK_SIZE/4 )
                len = BLOCK_SIZE/4;
            strncpy( ivRaw, argv[i+1], len );
            ivLength = len * 4;
            ivRaw[BLOCK_SIZE/4] = '\0';
        }
        else if( strcmp(arg, "-k") == 0 ){
            int len = strlen(argv[i+1]) + 1;
            if( len > MAX_KEY_ASCII_SIZE )
                len = MAX_KEY_ASCII_SIZE;
            strncpy( keyRaw, argv[i+1], len );
            keyLength = len * 4;
            keyRaw[MAX_KEY_ASCII_SIZE] = '\0';
        }

    }
    
    FILE *inputFile = fopen( inputFileName, "rb" );
    if( inputFile == NULL ){
        errno = ENOENT;
        perror( "Could not open input file" );
        exit(errno);
    }
    FILE *outputFile = fopen( outputFileName, "wb" );
    if( outputFile == NULL ){
        errno = ENOENT;
        perror( "Could not open output file" );
        safeExit(errno, inputFile, outputFile);
    }

    BYTE inputBuffer[BUFFER_SIZE], outputBuffer[BUFFER_SIZE];
    memset( inputBuffer, 0, BUFFER_SIZE );
    memset( outputBuffer, 0, BUFFER_SIZE );

    keyObject key;
    cipherObject cipher;
    if( initKey( &key, direction, keyLength, keyRaw ) || initCipher( &cipher, mode, ivRaw, ivLength ) )
        safeExit(-1, inputFile, outputFile);
    
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
            safeExit(2, inputFile, outputFile);
            break;
    }

    int readBytes = 0;
    while( (readBytes = fread( inputBuffer, sizeof(BYTE), BUFFER_SIZE, inputFile )) != 0 ){
        if( cipherFunction( &cipher, &key, BUFFER_SIZE*8, inputBuffer, outputBuffer ) ){
            perror("failed! invalid input size");
            safeExit(3, inputFile, outputFile);
        }

        #if DEBUG
            printf( "read bytes: %d\n", readBytes );
            printf( "read input:\n" );
            for( int i = 0; i < BUFFER_SIZE; i++ ){
                printf( "%02X ", inputBuffer[i] );
                if( (i+1) % 16 == 0 )
                    printf("\n");
            }
            printf("\n");
            printf( "written output:\n" );
            for( int i = 0; i < BUFFER_SIZE; i++ ){
                printf( "%02X ", outputBuffer[i] );
                if( (i+1) % 16 == 0 )
                    printf("\n");
            }
            printf("\n");
        #endif
        fwrite( outputBuffer, sizeof(BYTE), BUFFER_SIZE, outputFile );
        memset( inputBuffer, 0, BUFFER_SIZE );
    }

    fclose( inputFile );
    fclose( outputFile );
    
    return 0;
}

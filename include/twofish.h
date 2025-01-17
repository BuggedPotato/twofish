#pragma once
#include "types.h"
#include "enums.h"
void PHT( DWORD *a, DWORD *b );
int initKey( keyObject *key, direction direction, int keyLength, char *keyRaw );
int initCipher( cipherObject *cipher, mode mode, char *ivRaw, int ivLength );
int encryptBlock( cipherObject *cipher, keyObject *key, int inputLength, BYTE *input, BYTE *output );
int decryptBlock( cipherObject *cipher, keyObject *key, int inputLength, BYTE *input, BYTE *output );
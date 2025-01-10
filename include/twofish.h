#pragma once
#include "types.h"
#include "enums.h"
void PHT( DWORD *a, DWORD *b );
void keyInit( keyObject *key, direction direction, int keyLength, char *keyRaw );
int initCipher( cipherObject *cipher, mode mode );
int encryptBlock( cipherObject *cipher, keyObject *key, BYTE *input, int inputLength, BYTE *output );

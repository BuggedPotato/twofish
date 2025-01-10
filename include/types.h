#pragma once
#include<stdint.h>
#include "constants.h"

#define BYTE uint8_t
#define DWORD uint32_t
// union of 4 BYTE and DWORD
typedef union U_DWORD {
    DWORD dword;
    BYTE bytes[4];
} U_DWORD;

typedef struct key {
    direction direction;
    int keyLength;
    char keyRaw[MAX_KEY_ASCII_SIZE];
    DWORD keyDWords[MAX_KEY_SIZE / 32];
    DWORD roundKeys[ROUND_KEYS];
    U_DWORD sBoxKeys[MAX_KEY_SIZE / 64];
} keyObject;

typedef struct cipher {
    mode mode;
    // DWORD iv[BLOCK_SIZE/32];
} cipherObject;

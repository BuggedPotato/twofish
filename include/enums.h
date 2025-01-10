#pragma once
#include "types.h"

// encryption or decryption
typedef enum direction {
    ENCRYPT,
    DECRYPT
} direction;

// mode of operation
typedef enum mode {
    EBC,
    CBC
} mode;
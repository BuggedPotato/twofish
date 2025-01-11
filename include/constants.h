#pragma once

#define DEBUG 1
#define MAX_ROUNDS 16

#define BLOCK_SIZE 128
#define MAX_KEY_SIZE 256
#define MAX_KEY_ASCII_SIZE 64
#define ROUND_KEYS ( BLOCK_SIZE / 16 + 32 )

#define RK_CONST 0x02020202u
#define RK_CONST_SHIFT 0x01010101u
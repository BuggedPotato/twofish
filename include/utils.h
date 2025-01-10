#pragma once
#include "types.h"
BYTE ROR4(BYTE val, int shift);
BYTE ROL4(BYTE val, int shift);
DWORD ROR32(DWORD val, int shift);
DWORD ROL32(DWORD val, int shift);
int parseHex( int size, DWORD *dest, char *text );
U_DWORD reverseBytes( U_DWORD x );

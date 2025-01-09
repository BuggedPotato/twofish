#include "../include/types.h"
/*
circular bit shift
*/
BYTE ROR4(BYTE val, int shift){
    BYTE a = val >> shift;
    BYTE b = val << (sizeof(val) * 8 - shift);
    return a | b;
}

BYTE ROL4(BYTE val, int shift){
    BYTE a = val << shift;
    BYTE b = val >> (sizeof(val) * 8 - shift);
    return a | b;
}

DWORD ROR32(DWORD val, int shift){
    DWORD a = val >> shift;
    DWORD b = val << (sizeof(val) * 8 - shift);
    return a | b;
}

DWORD ROL32(DWORD val, int shift){
    DWORD a = val << shift;
    DWORD b = val >> (sizeof(val) * 8 - shift);
    return a | b;
}
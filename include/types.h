#pragma once
#include<stdint.h>
#define BYTE uint8_t
#define DWORD uint32_t
// union of 4 BYTE and DWORD
typedef union U_DWORD {
    DWORD dword;
    BYTE bytes[4];
} U_DWORD;
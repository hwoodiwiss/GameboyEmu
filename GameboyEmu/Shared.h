#pragma once

typedef unsigned char BYTE;
typedef signed char SBYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef void(*Op)(BYTE& reg, BYTE val1, BYTE val2);

const BYTE scrollingGraphic[48] = { 0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB,
0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E };


//Intel x86 is little endian
//So the high part (memory[addr + 1] gets left shifted 8 bit which results with 8 trailing 0's followed by the value held in the mem location
//EG. 2000 = 00001011 11100000
//Low: 0000 1011 High: 11100000
//High << 8: 00000000 11100000
//When Or'd with the low part, the lowpart value will now slot into the trailing zeros
//The implied end outcome for the OR operation is: 00001011 00000000 OR 00000000 11100000
//Which gives: 00001011 11100000
#define BytesToWord(low, high) (WORD)((low & 0xFF)|(high << 8))
#define WordToBytes(low, high, word) low = (BYTE)word; high = (BYTE)word >> 8

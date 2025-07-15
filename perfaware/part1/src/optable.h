#include "disassemble.h"
#include "global.h"
#include "hardware.h"
#include <stdio.h>
#include <stdlib.h>

void NOOP(int byte0);
void ADD_(int byte1);
void SUB_(int byte1);
void CMP_(int byte1);
void MOV_(int byte1);
void ADDA(int byte1);
void CMPA(int byte1);
void SUBA(int byte1);
void MOVA(int byte0);
void IMED(int byte1);
void MOVI(int byte);
void MOVR(int byte1);

/*********
 * JUMPS *
 *********/

void JNZ_(int byte1);
void JE__(int byte1);
void JL__(int byte1);
void JLE_(int byte1);
void JNLE(int byte1);
void JB__(int byte1);
void JBE_(int byte1);
void JNBE(int byte1);
void JP__(int byte1);
void JO__(int byte1);
void JS__(int byte1);
void JNE_(int byte1);
void JNL_(int byte1);
void JG__(int byte1);
void JNB_(int byte1);
void JA__(int byte1);
void JNP_(int byte1);
void JNO_(int byte1);
void JNS_(int byte1);
void LOOP(int byte1);
void LOPZ(int byte1);
void LPNZ(int byte1);
void JCXZ(int byte1);

/*************
 * JUMPS END *
 *************/

// clang-format off
void (*opTable[16][16])(int byte0);
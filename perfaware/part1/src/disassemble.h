#include "decode.h"
#include <string.h>

void disassembleImmediateToAccumulator(Instruction inst);
void disassembleRegisterMemoryToFromRegister(Instruction inst);
void disassembleMemoryModeNoDisp(Instruction inst);
void disassemblyMemoryMode8BitDisp(int rm, int8_t byte);
void disassembleMemoryMode16BitDisp(int rm, int byte1, int byte2);
void disassembleREG(int w, int reg);
void disassembleRM(Instruction inst);
void disassembleByte(int byte);
void disassembleBytes(int byte1, int byte2);
void initDisassembly();

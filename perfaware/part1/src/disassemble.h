#include <string.h>

void disassembleImmediateToAccumulator(int byte1);
void disassembleRegisterMemoryToFromRegister(int byte1);
void disassembleREG(int w, int reg);
void disassembleMemoryModeNoDisp(int rm);
void disassemblyMemoryMode8BitDisp(int rm, int8_t byte);
void disassembleMemoryMode16BitDisp(int rm, int byte1, int byte2);
void disassembleRM(int byte1, int byte2);
void disassembleByte(int byte);
void disassembleBytes(int byte1, int byte2);

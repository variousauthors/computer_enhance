// for 8-bit stores
#include "decode.h"

#define STORE_LOW(index, byte)                                                 \
  (REGISTERS[index] = (REGISTERS[index] & 0xFF00) | byte)
#define STORE_HIGH(index, byte)                                                \
  (REGISTERS[index] = (REGISTERS[index] & 0x00FF) | (byte) << 8)

// for 16-bit stores
#define STORE_WIDE(index, byte1, byte2)                                        \
  (REGISTERS[index] = (byte2 << 8) | byte1)

// for 16-bit stores
#define REGISTER_TO_REGISTER_STORE_WIDE(index1, index2)                        \
  (REGISTERS[index1] = REGISTERS[index2])

void moveImmediateToRegister(Instruction inst);
void moveRegisterToRegister(Instruction inst);
void mathRegisterMemoryAndRegisterToEither(Instruction inst);
void mathImmediateToRegister(Instruction inst);
void mathImmediateToAccumulator(Instruction inst);
void executeJump(Instruction inst);
void moveMemoryToRegister(Instruction inst);
void moveRegisterMemoryToFromRegister(Instruction inst);
void moveImmediateToRegisterMemory(Instruction inst);
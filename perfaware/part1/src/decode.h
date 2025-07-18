#ifndef DECODE_H
#define DECODE_H

#include "global.h"
#include "hardware.h"
#include <stdint.h>

typedef struct Instruction {
  // the first byte
  uint8_t op;

  unsigned d : 1;
  unsigned s : 1;
  unsigned w : 1;
  unsigned mod : 2;
  unsigned reg : 3;
  unsigned rm : 3;

  unsigned code : 3; // arithmetic code, ie 101 for sub, 000 for add

  // wasteful, but we can figure this out later
  uint8_t dispLo;
  uint8_t dispHi;
  uint8_t data1;
  uint8_t data2;
} Instruction;

typedef struct InstructionFormat {
  uint8_t D_FIELD_MASK;
  uint8_t D_FIELD_SHIFT;

  uint8_t W_FIELD_MASK;
  uint8_t W_FIELD_SHIFT;

  uint8_t MOD_FIELD_MASK;
  uint8_t MOD_FIELD_SHIFT;

  uint8_t REG_FIELD_MASK;
  uint8_t REG_FIELD_SHIFT;

  uint8_t RM_FIELD_MASK;
  uint8_t RM_FIELD_SHIFT;
} InstructionFormat;

// memory to accumulator / accumulator to memory
Instruction decodeMOVA(int byte0);

// immediate to register
Instruction decodeMOVI(int byte1);

// immediate to register/memory
Instruction decodeMOVR(int byte1);

Instruction decodeImmediateToAccumulatore(int byte1);
Instruction decodeRegisterMemoryToFromRegister(int byte1);
Instruction decodeImmediateToRegisterMemory(int byte1);
Instruction decodeJump(int byte1);

#endif
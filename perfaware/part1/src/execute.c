// for 8-bit stores
#include "execute.h"
#include "decode.h"
#include <stdlib.h>

#define STORE_LOW(index, byte)                                                 \
  (REGISTERS[index] = (REGISTERS[index] & 0xFF00) | byte)
#define STORE_HIGH(index, byte)                                                \
  (REGISTERS[index] = (REGISTERS[index] & 0x00FF) | (byte) << 8)

#define LOAD_HIGH(index) ((REGISTERS[index] & 0xFF00) >> 8)
#define LOAD_LOW(index) (REGISTERS[index] & 0x00FF)

// for 16-bit stores
#define STORE_WIDE(index, byte1, byte2)                                        \
  (REGISTERS[index] = (byte2 << 8) | byte1)

// for 16-bit stores
#define REGISTER_TO_REGISTER_STORE_WIDE(index1, index2)                        \
  (REGISTERS[index1] = REGISTERS[index2])

int getRegisterIndex(int w, int reg) { return w ? reg : (reg & 0b011); }

void updateRegister(int index, int value, int w, int high) {
  if (w) {
    REGISTERS[index] = value;
  } else {
    if (high) {
      STORE_HIGH(index, value);
    } else {
      STORE_LOW(index, value);
    }
  }
}

void storeImmediateToRegister(Instruction inst) {
  int index = getRegisterIndex(inst.w, inst.reg);
  int value = inst.w ? ((inst.data2) << 8 | inst.data1) : inst.data1;

  updateRegister(index, value, inst.w, inst.reg & 0b100);
}

void storeRegisterToRegister(Instruction inst) {
  // need to determine the width, the dest, and the source
  int dest = inst.d ? inst.reg : inst.rm;
  int destIndex = getRegisterIndex(inst.w, dest);
  int source = inst.d ? inst.rm : inst.reg;
  int sourceIndex = getRegisterIndex(inst.w, source);

  int value = inst.w           ? REGISTERS[sourceIndex]
              : source & 0b100 ? LOAD_HIGH(sourceIndex)
                               : LOAD_LOW(sourceIndex);

  fprintf(verboseChannel,
          "storeRegisterToResgiter -> dest: %02X, source: %02X, value: %02X\n",
          dest, source, value);

  updateRegister(destIndex, value, inst.w, dest & 0b100);
}

void mathRegisterMemoryAndRegisterToEither(Instruction inst) {
  int dest = inst.d ? inst.reg : inst.rm;
  int destIndex = getRegisterIndex(inst.w, dest);
  int source = inst.d ? inst.rm : inst.reg;
  int sourceIndex = getRegisterIndex(inst.w, source);

  int sourceValue = inst.w           ? REGISTERS[sourceIndex]
                    : source & 0b100 ? LOAD_HIGH(sourceIndex)
                                     : LOAD_LOW(sourceIndex);

  int destValue = inst.w         ? REGISTERS[destIndex]
                  : dest & 0b100 ? LOAD_HIGH(destIndex)
                                 : LOAD_LOW(destIndex);

  switch (inst.code) {
  case ALU_ADD: {
    destValue += sourceValue;
    break;
  }
  case ALU_SUB: {
    destValue -= sourceValue;
    break;
  }
  case ALU_CMP: {
    int tmp = destValue - sourceValue;
    break;
  }
  default:
    fprintf(stderr, "unknown alu code %02X\n", inst.code);
    exit(1);
    break;
  }

  updateRegister(destIndex, destValue, inst.w, dest & 0b100);
}

int getImmediateValue(Instruction inst) {

  switch (inst.s | inst.w) {
  case 0b00: {
    return inst.data1;
  }
  case 0b01: {
    return (inst.data2 << 8) | inst.data1;
  }
  case 0b10: {
    // redundant? but it's in the table
    return inst.data1;
  }
  case 0b11: {
    // sign extended 8-bit data to 16-bits
    return (inst.data1 & 0x80) ? (0xFF00 | inst.data1) : inst.data1;
  }
  default:
    fprintf(stderr, "unknown values for s|w %02X\n", inst.code);
    exit(1);
    break;
  }
}

int performMath(Instruction inst, uint16_t destValue, uint16_t sourceValue) {
  uint16_t tmp;
  switch (inst.code) {
  case ALU_ADD: {
    tmp = destValue + sourceValue;
    break;
  }
  case ALU_SUB: {
    tmp = destValue - sourceValue;
    break;
  }
  case ALU_CMP: {
    tmp = destValue - sourceValue;
    break;
  }
  default:
    fprintf(stderr, "unknown alu code %02X\n", inst.code);
    exit(1);
    break;
  }

  FLAGS.ZF = tmp == 0;
  FLAGS.SF = (tmp & 0x8000) != 0;
  printf("wat %04X\n", tmp);

  return tmp;
}

void mathImmediateToRegister(Instruction inst) {
  int dest = inst.rm;
  int destIndex = getRegisterIndex(inst.w, dest);

  int destValue = inst.w         ? REGISTERS[destIndex]
                  : dest & 0b100 ? LOAD_HIGH(destIndex)
                                 : LOAD_LOW(destIndex);

  int sourceValue = getImmediateValue(inst);

  destValue = performMath(inst, destValue, sourceValue);

  updateRegister(destIndex, destValue, inst.w, dest & 0b100);
}

void mathImmediateToAccumulator(Instruction inst) {
  int dest = 0;
  int destIndex = getRegisterIndex(inst.w, dest);

  int destValue = inst.w         ? REGISTERS[destIndex]
                  : dest & 0b100 ? LOAD_HIGH(destIndex)
                                 : LOAD_LOW(destIndex);

  int sourceValue = getImmediateValue(inst);

  destValue = performMath(inst, destValue, sourceValue);

  updateRegister(destIndex, destValue, inst.w, dest & 0b100);
}

void executeJump(Instruction inst) {
  switch (inst.op) {
  case JUMP_JNZ: {
    if (!FLAGS.ZF) {
      printf("BEFORE %04X\n", IP);
      printf("IP-INC8 %02X\n", inst.data1);
      printf("IP-INC8 %d\n", (int16_t)(int8_t)inst.data1);
      IP += (int16_t)(int8_t)inst.data1;
      printf("AFTER %04X\n", IP);
    }
    break;
  }

  default:
    break;
  }
}
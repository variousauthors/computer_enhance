// for 8-bit stores
#include "execute.h"
#include "decode.h"
#include "hardware.h"
#include <stdlib.h>

int16_t RELEVANT_ADDRESSED[1024 * 1024] = {0};

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

void performMath(Instruction inst, int *destValue, uint16_t sourceValue);
int getRegisterIndex(int w, int reg) { return w ? reg : (reg & 0b011); }

void updateMemory(int index, int value, int w) {
  fprintf(verboseChannel, "updateMemory w: %d 0x%04X 0x%02X\n", w, index,
          value);
  if (w) {
    // store two bytes
    RELEVANT_ADDRESSED[index] = index;
    MEMORY[index++] = value & 0x00FF;
    RELEVANT_ADDRESSED[index] = index;
    MEMORY[index] = value & 0xFF00;
  } else {
    MEMORY[index] = value;
    RELEVANT_ADDRESSED[index] = index;
  }
}

int loadMemory(int index, int w) {
  fprintf(verboseChannel, "loadMemory %d\n", index);
  if (w) {
    // store two bytes
    int lo = MEMORY[index++];
    int hi = MEMORY[index];

    return (hi << 8) | lo;
  } else {
    return MEMORY[index];
  }
}

int loadRegister(int index, int w, int high) {
  fprintf(verboseChannel, "loadRegister %d\n", index);
  if (w) {
    return REGISTERS[index];
  } else {
    if (high) {
      return LOAD_HIGH(index);
    } else {
      return LOAD_LOW(index);
    }
  }
}

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

void moveImmediateToRegister(Instruction inst) {
  int index = getRegisterIndex(inst.w, inst.reg);
  int value = inst.w ? ((inst.data2) << 8 | inst.data1) : inst.data1;

  updateRegister(index, value, inst.w, inst.reg & 0b100);
}

int getDisplacement(Instruction inst) {
  int disp8 = inst.dispLo;
  int disp16 = (inst.dispHi << 8) | inst.dispLo;

  switch (inst.mod) {
  case MEMORY_MODE_NO_DISP: {
    return inst.rm == 0b110 ? disp16 : 0;
  }
  case MEMORY_MODE_8_BIT_DISP: {
    return disp8;
  }
  case MEMORY_MODE_16_BIT_DISP: {
    return disp16;
  }
  default:
    return 0;
  }
}

// returns an index into memory
int getMemoryIndex(Instruction inst) {
  // we don't care in here whether this is dest or source, we just
  // calculate the address using rm and disp
  int base = 0;
  int offset = 0;
  int disp = getDisplacement(inst);

  /*     | 00          | 01               | 10                |
   * ----+-------------+------------------+-------------------|
   * 000 | (BX) + (SI) | (BX) + (SI) + D8 | (BX) + (SI) + D16 |
   * 001 | (BX) + (DI) | (BX) + (DI) + D8 | (BX) + (DI) + D16 |
   * 010 | (BP) + (SI) | (BP) + (SI) + D8 | (BP) + (SI) + D16 |
   * 011 | (BP) + (DI) | (BP) + (DI) + D8 | (BP) + (DI) + D16 |
   * 100 | (SI)        | (SI) + D8        | (SI) + D16        |
   * 101 | (DI)        | (DI) + D8        | (DI) + D16        |
   * 110 | DIRECT ADDR | (BP) + D8        | (BP) + D16        |
   * 111 | (BX)        | (BX) + D8        | (BX) + D16        |
   * */

  if (inst.rm == 0b110) {
    // direct address
    base = 0;
    offset = 0;
  } else if (inst.rm == 0b111) {
    base = REGISTERS[INDEX_BX];
    offset = 0;
  } else {
    offset = inst.rm & 0b001 ? REGISTERS[INDEX_DI] : REGISTERS[INDEX_SI];
    base = inst.rm & 0b100   ? 0
           : inst.rm & 0b010 ? REGISTERS[INDEX_BP]
                             : REGISTERS[INDEX_BX];
  }

  return base + offset + disp;
}

void moveImmediateToRegisterMemory(Instruction inst) {
  int dest = inst.rm;
  int value = inst.w ? (inst.data2 << 8) | inst.data1 : inst.data1;

  int destIndex = inst.d || inst.mod == REGISTER_MODE
                      ? getRegisterIndex(inst.w, dest)
                      : getMemoryIndex(inst);

  fprintf(verboseChannel, "move -> dest: %s %d, value: %d\n",
          inst.d ? "reg" : "rm", destIndex, value);

  if (inst.mod == REGISTER_MODE) {
    updateRegister(destIndex, value, inst.w, dest & 0b100);
  } else {
    updateMemory(destIndex, value, inst.w);
  }
}

void moveRegisterMemoryToFromRegister(Instruction inst) {
  int dest = inst.d ? inst.reg : inst.rm;
  int source = inst.d ? inst.rm : inst.reg;

  int destIndex = inst.d || inst.mod == REGISTER_MODE
                      ? getRegisterIndex(inst.w, dest)
                      : getMemoryIndex(inst);

  int sourceIndex = !inst.d || inst.mod == REGISTER_MODE
                        ? getRegisterIndex(inst.w, source)
                        : getMemoryIndex(inst);

  fprintf(verboseChannel, "move -> dest: %s %d, source: %s %d \n",
          inst.d ? "reg" : "rm", destIndex, inst.d ? "rm" : "reg", sourceIndex);

  // now we load the value from source
  // and store it to dest
  int sourceValue = inst.d ? loadMemory(sourceIndex, inst.w)
                           : loadRegister(sourceIndex, inst.w, source & 0b100);

  // write to dest
  if (inst.d) {
    // register is dest
    updateRegister(destIndex, sourceValue, inst.w, dest & 0b100);
  } else {
    // memory is dest
    updateMemory(destIndex, sourceValue, inst.w);
  }
}

void moveRegisterToRegister(Instruction inst) {
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

  performMath(inst, &destValue, sourceValue);
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

void performMath(Instruction inst, int *destValue, uint16_t sourceValue) {
  uint16_t tmp;
  switch (inst.code) {
  case ALU_ADD: {
    tmp = *destValue + sourceValue;
    *destValue = tmp;
    break;
  }
  case ALU_SUB: {
    tmp = *destValue - sourceValue;
    *destValue = tmp;
    break;
  }
  case ALU_CMP: {
    tmp = *destValue - sourceValue;
    break;
  }
  default:
    fprintf(stderr, "unknown alu code %02X\n", inst.code);
    exit(1);
    break;
  }

  FLAGS.ZF = tmp == 0;
  FLAGS.SF = (tmp & 0x8000) != 0;
}

void mathImmediateToRegister(Instruction inst) {
  int dest = inst.rm;
  int destIndex = getRegisterIndex(inst.w, dest);

  int destValue = inst.w         ? REGISTERS[destIndex]
                  : dest & 0b100 ? LOAD_HIGH(destIndex)
                                 : LOAD_LOW(destIndex);

  int sourceValue = getImmediateValue(inst);

  performMath(inst, &destValue, sourceValue);

  updateRegister(destIndex, destValue, inst.w, dest & 0b100);
}

void mathImmediateToAccumulator(Instruction inst) {
  int dest = 0;
  int destIndex = getRegisterIndex(inst.w, dest);

  int destValue = inst.w         ? REGISTERS[destIndex]
                  : dest & 0b100 ? LOAD_HIGH(destIndex)
                                 : LOAD_LOW(destIndex);

  int sourceValue = getImmediateValue(inst);

  performMath(inst, &destValue, sourceValue);

  updateRegister(destIndex, destValue, inst.w, dest & 0b100);
}

void executeJump(Instruction inst) {
  switch (inst.op) {
  case JUMP_JNZ: {
    if (!FLAGS.ZF) {
      fprintf(verboseChannel, "BEFORE %04X\n", IP);
      fprintf(verboseChannel, "IP-INC8 %02X\n", inst.data1);
      fprintf(verboseChannel, "IP-INC8 %d\n", (int16_t)(int8_t)inst.data1);
      IP += (int16_t)(int8_t)inst.data1;
      fprintf(verboseChannel, "AFTER %04X\n", IP);
    }
    break;
  }

  default:
    break;
  }
}
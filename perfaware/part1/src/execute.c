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

void storeImmediateToRegister(Instruction inst) {
  int index = getRegisterIndex(inst.w, inst.reg);

  if (inst.w) {
    STORE_WIDE(index, inst.data1, inst.data2);
  } else {
    if (inst.reg & 0b100) {
      STORE_HIGH(index, inst.data1);
    } else {
      STORE_LOW(index, inst.data1);
    }
  }
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

  if (inst.w) {
    REGISTERS[destIndex] = value;
  } else {
    if (dest & 0b100) {
      STORE_HIGH(destIndex, value);
    } else {
      STORE_LOW(destIndex, value);
    }
  }
}
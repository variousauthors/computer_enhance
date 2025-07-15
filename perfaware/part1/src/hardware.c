#include "hardware.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*                       AX CX DX BC SP BP SI DI */
uint16_t REGISTERS[8] = {0, 0, 0, 0, 0, 0, 0, 0};

// for 8-bit stores
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

void registerStore8(int w, REG_ENCODING reg, uint8_t byte1) {
  switch (w << 3 | reg) {
  case AL: {
    STORE_LOW(AL, byte1);
    break;
  }
  case AH: {
    STORE_HIGH(0b100 | AH, byte1);
    break;
  }
  case BL: {
    STORE_LOW(BL, byte1);
    break;
  }
  case BH: {
    STORE_HIGH(0b100 | BH, byte1);
    break;
  }
  case CL: {
    STORE_LOW(CL, byte1);
    break;
  }
  case CH: {
    STORE_HIGH(0b100 | CH, byte1);
    break;
  }
  case DL: {
    STORE_LOW(DL, byte1);
    break;
  }
  case DH: {
    STORE_HIGH(0b100 | DH, byte1);
    break;
  }
  default:
    fprintf(stderr, "tries to store 8 bit into bad reg %02X\n", reg);
    exit(1);
    break;
  }
}

void registerStore16(int w, REG_ENCODING reg, uint8_t byte1, uint8_t byte2) {
  switch (w << 3 | reg) {
  case AX: {
    STORE_WIDE(reg & 0b0011, byte1, byte2);
    break;
  }
  case BX: {
    STORE_WIDE(reg & 0b011, byte1, byte2);
    break;
  }
  case CX: {
    STORE_WIDE(reg & 0b011, byte1, byte2);
    break;
  }
  case DX: {
    STORE_WIDE(reg & 0b011, byte1, byte2);
    break;
  }
  case SP: {
    STORE_WIDE(reg & 0b111, byte1, byte2);
    break;
  }
  case BP: {
    STORE_WIDE(reg & 0b111, byte1, byte2);
    break;
  }
  case SI: {
    STORE_WIDE(reg & 0b111, byte1, byte2);
    break;
  }
  case DI: {
    STORE_WIDE(reg & 0b111, byte1, byte2);
    break;
  }
  default:
    fprintf(stderr, "tries to store 16 bit into bad reg %02X\n", reg);
    exit(1);
    break;
  }
}

void registerToRegisterStore16(int w, REG_ENCODING reg1, REG_ENCODING reg2) {
  REGISTER_TO_REGISTER_STORE_WIDE(reg1, reg2);
}

void storeRegisterMemoryToFromRegister(int d, int w, int rm, int reg, int byte1,
                                       int byte2) {
  int a;
}
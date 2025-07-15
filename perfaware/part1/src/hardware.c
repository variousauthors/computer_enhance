#include "hardware.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uint16_t REGISTERS[8] = {0, 0, 0, 0, 0, 0, 0, 0};

// for 8-bit stores
#define STORE_LOW(index, byte)                                                 \
  (REGISTERS[index] = (REGISTERS[index] & 0xFF00) | byte)
#define STORE_HIGH(index, byte)                                                \
  (REGISTERS[index] = (REGISTERS[index] & 0x00FF) | (byte) << 8)

// for 16-bit stores
#define STORE_WIDE(index, byte1, byte2)                                        \
  (REGISTERS[index] = (byte2 << 8) | byte1)

void registerStore8(int w, REG_ENCODING reg, uint8_t byte1) {
  switch (w << 3 | reg) {
  case AL: {
    STORE_LOW(0, byte1);
    break;
  }
  case AH: {
    STORE_LOW(0, byte1);
    break;
  }
  case BL: {
    STORE_LOW(1, byte1);
    break;
  }
  case BH: {
    STORE_LOW(1, byte1);
    break;
  }
  case CL: {
    STORE_LOW(2, byte1);
    break;
  }
  case CH: {
    STORE_LOW(2, byte1);
    break;
  }
  case DL: {
    STORE_LOW(3, byte1);
    break;
  }
  case DH: {
    STORE_LOW(3, byte1);
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
    STORE_WIDE(0, byte1, byte2);
    break;
  }
  case BX: {
    STORE_WIDE(1, byte1, byte2);
    break;
  }
  case CX: {
    STORE_WIDE(2, byte1, byte2);
    break;
  }
  case DX: {
    STORE_WIDE(3, byte1, byte2);
    break;
  }
  case SP: {
    STORE_WIDE(4, byte1, byte2);
    break;
  }
  case BP: {
    STORE_WIDE(5, byte1, byte2);
    break;
  }
  case SI: {
    STORE_WIDE(6, byte1, byte2);
    break;
  }
  case DI: {
    STORE_WIDE(7, byte1, byte2);
    break;
  }
  default:
    fprintf(stderr, "tries to store 16 bit into bad reg %02X\n", reg);
    exit(1);
    break;
  }
}
#ifndef DECODE_H
#define DECODE_H

#include "global.h"
#include "hardware.h"
#include <stdint.h>

typedef struct Instruction {
  // the first byte
  uint8_t op;

  unsigned d : 1;
  unsigned w : 1;
  unsigned mod : 2;
  unsigned reg : 3;
  unsigned rm : 3;

  // wasteful, but we can figure this out later
  uint8_t dispLo;
  uint8_t dispHi;
  uint8_t data1;
  uint8_t data2;
} Instruction;

// register/memory to/from register
Instruction decodeMOV_(int byte1);

#endif
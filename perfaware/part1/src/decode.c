#include "decode.h"
#include "global.h"
#include "hardware.h"

// register/memory to/from register
Instruction decodeMOV_(int byte1) {
  Instruction inst = {0};

  int byte2 = nextByte();

  inst.op = byte1;
  inst.w = byte1 & BIT_W_MASK;
  inst.d = byte1 & BIT_D_MASK;
  inst.reg = (byte2 & REG_MASK) >> 3;
  inst.mod = (byte2 & 0b11000000) >> 6;
  inst.rm = (byte2 & RM_MASK);

  switch (inst.mod) {
  case MEMORY_MODE_NO_DISP: {
    if (inst.rm == 0b110) {
      inst.dispLo = nextByte();
      inst.dispHi = nextByte();
    }
    break;
  }
  case MEMORY_MODE_8_BIT_DISP: {
    inst.dispLo = nextByte();
    break;
  }
  case MEMORY_MODE_16_BIT_DISP: {
    inst.dispLo = nextByte();
    inst.dispHi = nextByte();
    break;
  }
  case REGISTER_MODE: {
    break;
  }
  default:
    break;
  }

  fprintf(verboseChannel,
          "MOV_: op: %02X, d: %01X, w: %01X, mod: %01X, reg: %01X, rm: %01X\n",
          inst.op, inst.d, inst.w, inst.mod, inst.reg, inst.rm);

  return inst;
}
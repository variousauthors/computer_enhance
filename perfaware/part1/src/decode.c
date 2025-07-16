#include "decode.h"
#include "global.h"
#include "hardware.h"

void debugInstruction(Instruction inst) {
  fprintf(verboseChannel,
          "INST: op: %02X, d: %01X, w: %01X, mod: %01X, reg: %01X, rm: %01X, "
          "dispLo: %02X, dispHi: %02X, data1: %02X, data2: %02X\n",
          inst.op, inst.d, inst.w, inst.mod, inst.reg, inst.rm, inst.dispLo,
          inst.dispHi, inst.data1, inst.data2);
}

// register/memory to/from register
Instruction decodeMOV_(int byte1) {
  Instruction inst = {0};

  int byte2 = nextByte();

  inst.op = byte1;
  inst.d = (byte1 & BIT_D_MASK) >> 1;
  inst.w = byte1 & BIT_W_MASK;
  inst.reg = (byte2 & REG_MASK) >> 3;
  inst.mod = (byte2 & 0b11000000) >> 6;
  inst.rm = (byte2 & RM_MASK);

  switch (inst.mod) {
  case MEMORY_MODE_NO_DISP: {
    if (inst.rm == 0b110) {
      fprintf(verboseChannel, "yes\n");
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

  debugInstruction(inst);

  return inst;
}

// memory to accumulator / accumulator to memory
Instruction decodeMOVA(int byte1) {
  Instruction inst = {0};

  fprintf(verboseChannel, "decodeMOVI %02X\n", byte1);

  inst.op = byte1;
  inst.d = (byte1 & BIT_D_MASK) >> 1;
  inst.w = (byte1 & BIT_W_MASK);
  inst.reg = 0;

  // kind of hacky, but we know we want [16-bit immediate]
  // and we know this does it
  inst.mod = 0;    // we always want MOVA to use mode 00
  inst.rm = 0b110; // with the special rm 110

  inst.dispLo = nextByte();
  inst.dispHi = nextByte();

  debugInstruction(inst);

  return inst;
}

// immediate to register
Instruction decodeMOVI(int byte1) {
  Instruction inst = {0};

  fprintf(verboseChannel, "decodeMOVI %02X\n", byte1);

  inst.op = byte1;
  inst.d = 0;
  inst.w = (byte1 & MOV_IMM_W_MASK) >> 3;
  inst.reg = (byte1 & MOV_IMM_REG_MASK);
  inst.mod = 0;
  inst.rm = 0;

  inst.data1 = nextByte();

  if (inst.w) {
    inst.data2 = nextByte();
  }

  debugInstruction(inst);

  return inst;
}

// immediate to register/memory
Instruction decodeMOVR(int byte1) {
  Instruction inst = {0};

  int byte2 = nextByte();

  fprintf(verboseChannel, "decodeMOVR %02X %02X\n", byte1, byte2);

  inst.op = byte1;
  inst.w = byte1 & BIT_W_MASK;
  inst.reg = 0;
  inst.mod = (byte2 & 0b11000000) >> 6;
  inst.rm = (byte2 & RM_MASK);

  switch (inst.mod) {
  case MEMORY_MODE_NO_DISP: {
    if (inst.rm == 0b110) {
      fprintf(verboseChannel, "yes\n");
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

  inst.data1 = nextByte();

  if (inst.w) {
    inst.data2 = nextByte();
  }

  debugInstruction(inst);

  return inst;
}

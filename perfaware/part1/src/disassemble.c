#include "disassemble.h"
#include "global.h"
#include "hardware.h"
#include <stdio.h>

void initDisassembly() { printf("bits 16\n"); }

void disassembleImmediateToAccumulator(Instruction inst) {
  // always accumulator reg = 0
  disassembleREG(inst.w, inst.reg);
  printf(", ");

  if (inst.w) {
    disassembleBytes(inst.data1, inst.data2);
  } else {
    disassembleByte(inst.data1);
  }

  printf("\n");
}

void disassembleRegisterMemoryToFromRegister(Instruction inst) {
  int w = inst.w;
  int d = inst.d;
  int reg = inst.reg;

  if (d) {
    // reg is dest
    disassembleREG(w, reg);
    printf(", ");
    disassembleRM(inst);
    printf("\n");
  } else {
    // reg is source
    disassembleRM(inst);
    printf(", ");
    disassembleREG(w, reg);
    printf("\n");
  }
}

// @param w - 1 bit
// @param reg - 3 bits
void disassembleREG(int w, int reg) {
  // fprintf(verboseChannel, "decodeREG -> w: %02X, reg: %02X\n", w, reg);

  switch ((w << 3) | (reg)) {
  case AL:
    printf("al");
    break;
  case CL:
    printf("cl");
    break;
  case DL:
    printf("dl");
    break;
  case BL:
    printf("bl");
    break;
  case AH:
    printf("ah");
    break;
  case CH:
    printf("ch");
    break;
  case DH:
    printf("dh");
    break;
  case BH:
    printf("bh");
    break;
  case AX:
    printf("ax");
    break;
  case CX:
    printf("cx");
    break;
  case DX:
    printf("dx");
    break;
  case BX:
    printf("bx");
    break;
  case SP:
    printf("sp");
    break;
  case BP:
    printf("bp");
    break;
  case SI:
    printf("si");
    break;
  case DI:
    printf("di");
    break;
  default:
    break;
  }
}

void disassembleMemoryModeNoDisp(Instruction inst) {
  fprintf(verboseChannel, "decodeMemoryModeNoDisp -> rm: %02X\n", inst.rm);

  int16_t disp = -1;
  if (inst.rm == 0b110) {
    int byte0 = inst.dispLo;
    int byte1 = inst.dispHi;
    disp = (byte1 << 8) | byte0;
    fprintf(verboseChannel, "  -> disp: %04X\n", disp);
  }

  switch (inst.rm) {
  case BX_SI:
    printf("[bx + si]");
    break;
  case BX_DI:
    printf("[bx + di]");
    break;
  case BP_SI:
    printf("[bp + si]");
    break;
  case BP_DI:
    printf("[bp + di]");
    break;
  case SI_:
    printf("[si]");
    break;
  case DI_:
    printf("[di]");
    break;
  case BP_:
    disp <= 0 ? printf("[bp]") : printf("[%d]", disp);
    break;
  case BX_:
    printf("[bx]");
    break;
  default:
    break;
  }
}

void disassemblyMemoryMode8BitDisp(int rm, int8_t byte) {
  switch (rm) {
  case BX_SI:
    printf("[bx + si + %d]", byte);
    break;
  case BX_DI:
    printf("[bx + di + %d]", byte);
    break;
  case BP_SI:
    printf("[bp + si + %d]", byte);
    break;
  case BP_DI:
    printf("[bp + di + %d]", byte);
    break;
  case SI_:
    printf("[si + %d]", byte);
    break;
  case DI_:
    printf("[di + %d]", byte);
    break;
  case BP_:
    byte != 0 ? printf("[bp + %d]", byte) : printf("[bp]");
    break;
  case BX_:
    printf("[bx + %d]", byte);
    break;
  default:
    break;
  }
}

void disassembleMemoryMode16BitDisp(int rm, int byte1, int byte2) {
  int disp = (byte2 << 8) | byte1;

  switch (rm) {
  case BX_SI:
    printf("[bx + si + %d]", disp);
    break;
  case BX_DI:
    printf("[bx + di + %d]", disp);
    break;
  case BP_SI:
    printf("[bp + si + %d]", disp);
    break;
  case BP_DI:
    printf("[bp + di + %d]", disp);
    break;
  case SI_:
    printf("[si + %d]", disp);
    break;
  case DI_:
    printf("[di + %d]", disp);
    break;
  case BP_:
    disp != 0 ? printf("[bp + %d]", disp) : printf("[bp]");
    break;
  case BX_:
    printf("[bx + %d]", disp);
    break;
  default:
    break;
  }
}

void disassembleRM(Instruction inst) {
  int mod = inst.mod;
  int w = inst.w;
  int rm = inst.rm;

  fprintf(verboseChannel, "decodeRM -> %02X %02X %02X\n", w, mod, rm);

  switch (mod) {
  case MEMORY_MODE_NO_DISP: {
    disassembleMemoryModeNoDisp(inst);
    break;
  }
  case MEMORY_MODE_8_BIT_DISP: {
    disassemblyMemoryMode8BitDisp(rm, inst.dispLo);
    break;
  }
  case MEMORY_MODE_16_BIT_DISP: {
    disassembleMemoryMode16BitDisp(rm, inst.dispLo, inst.dispHi);
    break;
  }
  case REGISTER_MODE: {
    disassembleREG(w, rm);
    break;
  }
  default:
    break;
  }
}

void disassembleByte(int byte) { printf("%d", byte); }

void disassembleBytes(int byte1, int byte2) {
  printf("%d", (byte2 << 8) | byte1);
}

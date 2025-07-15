#include "optable.h"
#include "decode.h"
#include "disassemble.h"
#include "global.h"
#include "hardware.h"
#include <stdio.h>
#include <stdlib.h>

#define nextByte() (fgetc(source))

void NOOP(int byte0) {
  printf("NO_OP %02X\n", byte0);
  return;
}

// add register/memory to/from register
void ADD_(int byte1) {
  fprintf(verboseChannel, "ADD -> %02X\n", byte1);
  printf("add ");

  disassembleRegisterMemoryToFromRegister(byte1);
}

// add register/memory to/from register
void SUB_(int byte1) {
  fprintf(verboseChannel, "SUB -> %02X\n", byte1);
  printf("sub ");

  disassembleRegisterMemoryToFromRegister(byte1);
}

// compare register/memory with register
void CMP_(int byte1) {
  fprintf(verboseChannel, "CMP_ -> %02X\n", byte1);
  printf("cmp ");

  disassembleRegisterMemoryToFromRegister(byte1);
}

// register/memory to/from register
void MOV_(int byte1) {
  fprintf(verboseChannel, "MOVR -> %02X\n", byte1);
  printf("mov ");

  Instruction inst = decodeMOV_(byte1);

  disassembleRegisterMemoryToFromRegister(byte1);
}

// add immediate to accumulator
void ADDA(int byte1) {
  printf("add ");
  fprintf(verboseChannel, "ADDA -> %02X\n", byte1);

  disassembleImmediateToAccumulator(byte1);
}

// immediate with accumulator
void CMPA(int byte1) {
  printf("cmp ");
  fprintf(verboseChannel, "CMPA -> %02X\n", byte1);

  disassembleImmediateToAccumulator(byte1);
}

// add immediate to register/memory
// confusing: (from) immediate value subtracted _from_ (ie store to)
// register/memory
void SUBA(int byte1) {
  printf("sub ");
  fprintf(verboseChannel, "SUBA -> %02X\n", byte1);

  disassembleImmediateToAccumulator(byte1);
}

// memory to accumulator / accumulator to memory
void MOVA(int byte0) {
  printf("mov ");

  int d = byte0 & BIT_D_MASK;
  int w = byte0 & BIT_W_MASK;

  fprintf(verboseChannel, "MOVA d: %02X w: %02X\n", d, w);

  // logic for the d bit is inverted for
  // immediate to acc / acc to immediate
  // 1010000w - AX is dest
  // 1010001w - AX is source
  if (!d) {
    // always accumulator reg = 0
    disassembleREG(w, 0);
    printf(", ");
    // kind of hacky, but we know we want [16-bit immediate]
    // and we know this does it
    disassembleMemoryModeNoDisp(BP_);
  } else {
    // kind of hacky, but we know we want [16-bit immediate]
    // and we know this does it
    disassembleMemoryModeNoDisp(BP_);
    printf(", ");
    // always accumulator reg = 0
    disassembleREG(w, 0);
  }

  printf("\n");

  return;
}

// immediate mode add, sub, cmp
void IMED(int byte1) {
  int byte2 = nextByte();
  fprintf(verboseChannel, "IMED -> %02X %02X\n", byte1, byte2);

  int code = (byte2 & IMED_CODE_MASK) >> 3;

  fprintf(verboseChannel, "IMED -> code: %02X\n", code);

  switch (code) {
  case IMED_ADD:
    printf("add ");
    break;
  case IMED_SUB:
    printf("sub ");
    break;
  case IMED_CMP:
    printf("cmp ");
    break;
  default:
    fprintf(stderr, "unknown imed code %02X\n", code);
    exit(1);
    break;
  }

  int s = byte1 & IMED_SIGN_EXT_MASK;
  int w = byte1 & BIT_W_MASK;

  if (w) {
    printf("word ");
  } else {
    printf("byte ");
  }

  disassembleRM(byte1, byte2);
  printf(", ");

  switch (s | w) {
  case 0b00: {
    disassembleByte(nextByte());
    break;
  }
  case 0b01: {
    disassembleBytes(nextByte(), nextByte());
    break;
  }
  case 0b10: {
    // redundant? but it's in the table
    disassembleByte(nextByte());
    break;
  }
  case 0b11: {
    // sign extended 8-bit data to 16-bits
    int byte3 = nextByte();
    disassembleBytes(byte3 & 0x00FF, (byte3 & 0xFF00) >> 8);
    break;
  }
  default:
    fprintf(stderr, "unknown values for s|w %02X\n", code);
    exit(1);
    break;
  }

  printf("\n");
}

// immediate to register
void MOVI(int byte) {
  fprintf(verboseChannel, "MOVI\n");
  printf("mov ");

  int w = (byte & MOV_IMM_W_MASK) >> 3;
  int reg = (byte & MOV_IMM_REG_MASK);

  disassembleREG(w, reg);
  printf(", ");

  if (byte & MOV_IMM_W_MASK) {
    int byte1 = nextByte();
    int byte2 = nextByte();
    disassembleBytes(byte1, byte2);

    if (exec) {
      registerStore16(w, reg, byte1, byte2);
    }
  } else {
    int byte1 = nextByte();
    disassembleByte(byte1);

    if (exec) {
      registerStore8(w, reg, byte1);
    }
  }

  printf("\n");
}

// immediate to register/memory
void MOVR(int byte1) {
  int byte2 = nextByte();
  // byte1 byte2 (DISP_LO) (DISP_HI) data (data if w = 1)
  fprintf(verboseChannel, "MOVK -> %02X %02X\n", byte1, byte2);
  printf("mov ");

  disassembleRM(byte1, byte2);
  printf(", ");

  if (byte1 & MOV_W_MASK) {
    printf("word ");
    disassembleBytes(nextByte(), nextByte());
  } else {
    printf("byte ");
    disassembleByte(nextByte());
  }

  printf("\n");
}

/*********
 * JUMPS *
 *********/

#define JUMP(op)                                                               \
  do {                                                                         \
    printf("%s ", op);                                                         \
    disassembleByte(nextByte());                                               \
    printf("\n");                                                              \
  } while (0)

void JNZ_(int byte1) { JUMP("jnz"); }
void JE__(int byte1) { JUMP("je"); }
void JL__(int byte1) { JUMP("jl"); }
void JLE_(int byte1) { JUMP("jle"); }
void JNLE(int byte1) { JUMP("jnle"); }
void JB__(int byte1) { JUMP("jb"); }
void JBE_(int byte1) { JUMP("jbe"); }
void JNBE(int byte1) { JUMP("jnbe"); }
void JP__(int byte1) { JUMP("jp"); }
void JO__(int byte1) { JUMP("jo"); }
void JS__(int byte1) { JUMP("js"); }
void JNE_(int byte1) { JUMP("jne"); }
void JNL_(int byte1) { JUMP("jnl"); }
void JG__(int byte1) { JUMP("jg"); }
void JNB_(int byte1) { JUMP("jnb"); }
void JA__(int byte1) { JUMP("ja"); }
void JNP_(int byte1) { JUMP("jnp"); }
void JNO_(int byte1) { JUMP("jno"); }
void JNS_(int byte1) { JUMP("jns"); }
void LOOP(int byte1) { JUMP("loop"); }
void LOPZ(int byte1) { JUMP("loopz"); }
void LPNZ(int byte1) { JUMP("loopnz"); }
void JCXZ(int byte1) { JUMP("jcxz"); }

/*************
 * JUMPS END *
 *************/

// clang-format off
void (*opTable[16][16])(int byte0) = {
/* hi\lo  0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F */
/* 0 */ {ADD_, ADD_, ADD_, ADD_, ADDA, ADDA, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP},
/* 1 */ {NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP},
/* 2 */ {NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, SUB_, SUB_, SUB_, SUB_, SUBA, SUBA, NOOP, NOOP},
/* 3 */ {NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, CMP_, CMP_, CMP_, CMP_, CMPA, CMPA, NOOP, NOOP},
/* 4 */ {NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP},
/* 5 */ {NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP},
/* 6 */ {NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP},
/* 7 */ {JO__, JNO_, JB__, JNB_, JE__, JNZ_, JBE_, JNBE, JS__, JNS_, JP__, JNP_, JL__, JNL_, JLE_, JNLE},
/* 8 */ {IMED, IMED, IMED, IMED, NOOP, NOOP, NOOP, NOOP, MOV_, MOV_, MOV_, MOV_, NOOP, NOOP, NOOP, NOOP},
/* 9 */ {NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP},
/* A */ {MOVA, MOVA, MOVA, MOVA, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP},
/* B */ {MOVI, MOVI, MOVI, MOVI, MOVI, MOVI, MOVI, MOVI, MOVI, MOVI, MOVI, MOVI, MOVI, MOVI, MOVI, MOVI},
/* C */ {NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, MOVR, MOVR, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP},
/* D */ {NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP},
/* E */ {LPNZ, LOPZ, LOOP, JCXZ, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP},
/* F */ {NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP},
};
// clang-format on

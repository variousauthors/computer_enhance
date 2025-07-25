#include "optable.h"
#include "decode.h"
#include "disassemble.h"
#include "execute.h"
#include "global.h"
#include "hardware.h"
#include <stdio.h>
#include <stdlib.h>

void NOOP(int byte0) {
  fprintf(verboseChannel, "hit NOOP while parsing %02X\n", byte0);
  exit(1);
  return;
}

// add register/memory to/from register
void ADD_(int byte1) {
  fprintf(verboseChannel, "ADD_ -> %02X\n", byte1);
  printf("add ");

  Instruction inst = decodeRegisterMemoryToFromRegister(byte1);
  disassembleRegisterMemoryToFromRegister(inst);

  if (exec) {
    mathRegisterMemoryAndRegisterToEither(inst);
  }
}

// add register/memory to/from register
void SUB_(int byte1) {
  fprintf(verboseChannel, "SUB_ -> %02X\n", byte1);
  printf("sub ");

  Instruction inst = decodeRegisterMemoryToFromRegister(byte1);
  disassembleRegisterMemoryToFromRegister(inst);

  if (exec) {
    mathRegisterMemoryAndRegisterToEither(inst);
  }
}

// compare register/memory with register
void CMP_(int byte1) {
  fprintf(verboseChannel, "CMP_ -> %02X\n", byte1);
  printf("cmp ");

  Instruction inst = decodeRegisterMemoryToFromRegister(byte1);
  disassembleRegisterMemoryToFromRegister(inst);

  if (exec) {
    mathRegisterMemoryAndRegisterToEither(inst);
  }
}

// register/memory to/from register
void MOV_(int byte1) {
  fprintf(verboseChannel, "MOV_ -> %02X\n", byte1);
  printf("mov ");

  Instruction inst = decodeRegisterMemoryToFromRegister(byte1);
  disassembleRegisterMemoryToFromRegister(inst);

  if (exec) {
    moveRegisterMemoryToFromRegister(inst);
  }
}

// add immediate to accumulator
void ADDA(int byte1) {
  printf("add ");
  fprintf(verboseChannel, "ADDA -> %02X\n", byte1);

  Instruction inst = decodeImmediateToAccumulatore(byte1);
  disassembleImmediateToAccumulator(inst);

  if (exec) {
    mathImmediateToAccumulator(inst);
  }
}

// immediate with accumulator
void CMPA(int byte1) {
  printf("cmp ");
  fprintf(verboseChannel, "CMPA -> %02X\n", byte1);

  Instruction inst = decodeImmediateToAccumulatore(byte1);
  disassembleImmediateToAccumulator(inst);

  if (exec) {
    mathImmediateToAccumulator(inst);
  }
}

// add immediate to register/memory
// confusing: (from) immediate value subtracted _from_ (ie store to)
// register/memory
void SUBA(int byte1) {
  printf("sub ");
  fprintf(verboseChannel, "SUBA -> %02X\n", byte1);

  Instruction inst = decodeImmediateToAccumulatore(byte1);
  disassembleImmediateToAccumulator(inst);

  if (exec) {
    mathImmediateToAccumulator(inst);
  }
}

// memory to accumulator / accumulator to memory
void MOVA(int byte0) {
  printf("mov ");

  Instruction inst = decodeMOVA(byte0);

  fprintf(verboseChannel, "MOVA d: %02X w: %02X\n", inst.d, inst.w);

  // logic for the d bit is inverted for
  // immediate to acc / acc to immediate
  // 1010000w - AX is dest
  // 1010001w - AX is source
  if (!inst.d) {
    // always accumulator reg = 0
    disassembleREG(inst.w, inst.reg);
    printf(", ");
    disassembleRM(inst);
  } else {
    disassembleRM(inst);
    printf(", ");
    // always accumulator reg = 0
    disassembleREG(inst.w, inst.reg);
  }

  printf("\n");

  return;
}

// immediate mode add, sub, cmp
void IMED(int byte1) {
  Instruction inst = decodeImmediateToRegisterMemory(byte1);

  fprintf(verboseChannel, "IMED -> %02X %02X\n", inst.dispLo, inst.dispHi);
  fprintf(verboseChannel, "IMED -> code: %02X\n", inst.code);

  switch (inst.code) {
  case ALU_ADD:
    printf("add ");
    break;
  case ALU_SUB:
    printf("sub ");
    break;
  case ALU_CMP:
    printf("cmp ");
    break;
  default:
    fprintf(stderr, "unknown imed code %02X\n", inst.code);
    exit(1);
    break;
  }

  if (inst.w) {
    printf("word ");
  } else {
    printf("byte ");
  }

  disassembleRM(inst);
  printf(", ");

  switch (inst.s | inst.w) {
  case 0b00: {
    disassembleByte(inst.data1);
    break;
  }
  case 0b01: {
    disassembleBytes(inst.data1, inst.data2);
    break;
  }
  case 0b10: {
    // redundant? but it's in the table
    disassembleByte(inst.data1);
    break;
  }
  case 0b11: {
    // sign extended 8-bit data to 16-bits
    disassembleBytes(inst.data1, inst.data1 & 0x80 ? 0xFF : 0x00);
    break;
  }
  default:
    fprintf(stderr, "unknown values for s|w %02X\n", inst.code);
    exit(1);
    break;
  }

  if (exec) {
    mathImmediateToRegister(inst);
  }

  printf("\n");
}

// immediate to register
void MOVI(int byte) {
  Instruction inst = decodeMOVI(byte);
  fprintf(verboseChannel, "MOVI\n");
  printf("mov ");

  disassembleREG(inst.w, inst.reg);
  printf(", ");

  if (inst.w) {
    disassembleBytes(inst.data1, inst.data2);

  } else {
    disassembleByte(inst.data1);
  }

  if (exec) {
    moveImmediateToRegister(inst);
  }

  printf("\n");
}

// immediate to register/memory
void MOVR(int byte1) {
  Instruction inst = decodeMOVR(byte1);
  printf("mov ");

  disassembleRM(inst);
  printf(", ");

  if (inst.w) {
    printf("word ");
    disassembleBytes(inst.data1, inst.data2);
  } else {
    printf("byte ");
    disassembleByte(inst.data1);
  }

  if (exec) {
    moveImmediateToRegisterMemory(inst);
  }

  printf("\n");
}

/*********
 * JUMPS *
 *********/

#define JUMP(op, byte1)                                                        \
  do {                                                                         \
    Instruction inst = decodeJump(byte1);                                      \
    printf("%s ", op);                                                         \
    disassembleByte(inst.data1);                                               \
    printf("\n");                                                              \
    if (exec) {                                                                \
      executeJump(inst);                                                       \
    };                                                                         \
  } while (0)

void JNZ_(int byte1) { JUMP("jnz", byte1); }
void JE__(int byte1) { JUMP("je", byte1); }
void JL__(int byte1) { JUMP("jl", byte1); }
void JLE_(int byte1) { JUMP("jle", byte1); }
void JNLE(int byte1) { JUMP("jnle", byte1); }
void JB__(int byte1) { JUMP("jb", byte1); }
void JBE_(int byte1) { JUMP("jbe", byte1); }
void JNBE(int byte1) { JUMP("jnbe", byte1); }
void JP__(int byte1) { JUMP("jp", byte1); }
void JO__(int byte1) { JUMP("jo", byte1); }
void JS__(int byte1) { JUMP("js", byte1); }
void JNE_(int byte1) { JUMP("jne", byte1); }
void JNL_(int byte1) { JUMP("jnl", byte1); }
void JG__(int byte1) { JUMP("jg", byte1); }
void JNB_(int byte1) { JUMP("jnb", byte1); }
void JA__(int byte1) { JUMP("ja", byte1); }
void JNP_(int byte1) { JUMP("jnp", byte1); }
void JNO_(int byte1) { JUMP("jno", byte1); }
void JNS_(int byte1) { JUMP("jns", byte1); }
void LOOP(int byte1) { JUMP("loop", byte1); }
void LOPZ(int byte1) { JUMP("loopz", byte1); }
void LPNZ(int byte1) { JUMP("loopnz", byte1); }
void JCXZ(int byte1) { JUMP("jcxz", byte1); }

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

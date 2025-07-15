#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint16_t *REGISTERS[8] = {0, 0, 0, 0, 0, 0, 0, 0};

#define BIT_D_MASK 0b00000010
#define BIT_W_MASK 0b00000001
#define REG_MASK 0b00111000

#define IMED_SIGN_EXT_MASK BIT_D_MASK
#define IMED_CODE_MASK REG_MASK

#define MOV_W_MASK 0b00000001
#define MOV_D_MASK 0b00000010
#define MOV_REG_MASK 0b00111000
#define MOV_RM_MASK 0b00000111
#define MOV_MOD_MASK 0b11000000

#define MOV_IMM_W_MASK 0b00001000
#define MOV_IMM_REG_MASK 0b00000111

FILE *source;

uint8_t exec = 0;
uint8_t verbose = 0;

FILE *verboseChannel = 0;

#define nextByte() (fgetc(source))

void disassembleREG(int w, int reg);
void disassembleRM(int byte1, int byte2);
void disassembleMemoryModeNoDisp(int rm);
void disassembleByte(int byte);
void disassembleBytes(int byte1, int byte2);

enum MOD {
  MEMORY_MODE_NO_DISP, // DISP when R/M is 110
  MEMORY_MODE_8_BIT_DISP,
  MEMORY_MODE_16_BIT_DISP,
  REGISTER_MODE,
};

enum REG_ENCODING {
  AL,
  CL,
  DL,
  BL,
  AH,
  CH,
  DH,
  BH,
  AX,
  CX,
  DX,
  BX,
  SP,
  BP,
  SI,
  DI,
};

enum RM_ENCODING {
  BX_SI,
  BX_DI,
  BP_SI,
  BP_DI,
  SI_,
  DI_,
  BP_, // DIRECT_ACCESS when mod 00
  BX_,
};

enum IMED_CODE {
  IMED_ADD,
  IMED_OR,
  IMED_ADC,
  IMED_SBB,
  IMED_AND,
  IMED_SUB,
  IMED_XOR,
  IMED_CMP,
};

void decodeImmediateToAccumulator(int byte1) {
  int w = byte1 & BIT_W_MASK;

  // always accumulator reg = 0
  disassembleREG(w, 0);
  printf(", ");

  if (w) {
    disassembleBytes(nextByte(), nextByte());
  } else {
    disassembleByte(nextByte());
  }

  printf("\n");
}

void decodeRegisterMemoryToFromRegister(int byte1) {
  int byte2 = nextByte();

  int w = byte1 & BIT_W_MASK;
  int d = byte1 & BIT_D_MASK;
  int reg = (byte2 & REG_MASK) >> 3;

  if (d) {
    // reg is dest
    disassembleREG(w, reg);
    printf(", ");
    disassembleRM(byte1, byte2);
    printf("\n");
  } else {
    // reg is source
    disassembleRM(byte1, byte2);
    printf(", ");
    disassembleREG(w, reg);
    printf("\n");
  }
}

void NOOP(int byte0) {
  printf("NO_OP %02X\n", byte0);
  return;
}

// add register/memory to/from register
void ADD_(int byte1) {
  fprintf(verboseChannel, "ADD -> %02X\n", byte1);
  printf("add ");

  decodeRegisterMemoryToFromRegister(byte1);
}

// add register/memory to/from register
void SUB_(int byte1) {
  fprintf(verboseChannel, "SUB -> %02X\n", byte1);
  printf("sub ");

  decodeRegisterMemoryToFromRegister(byte1);
}

// compare register/memory with register
void CMP_(int byte1) {
  fprintf(verboseChannel, "CMP_ -> %02X\n", byte1);
  printf("cmp ");

  decodeRegisterMemoryToFromRegister(byte1);
}

// register/memory to/from register
void MOV_(int byte1) {
  fprintf(verboseChannel, "MOVR -> %02X\n", byte1);
  printf("mov ");

  decodeRegisterMemoryToFromRegister(byte1);
}

// add immediate to accumulator
void ADDA(int byte1) {
  printf("add ");
  fprintf(verboseChannel, "ADDA -> %02X\n", byte1);

  decodeImmediateToAccumulator(byte1);
}

// immediate with accumulator
void CMPA(int byte1) {
  printf("cmp ");
  fprintf(verboseChannel, "CMPA -> %02X\n", byte1);

  decodeImmediateToAccumulator(byte1);
}

// add immediate to register/memory
// confusing: (from) immediate value subtracted _from_ (ie store to)
// register/memory
void SUBA(int byte1) {
  printf("sub ");
  fprintf(verboseChannel, "SUBA -> %02X\n", byte1);

  decodeImmediateToAccumulator(byte1);
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
    disassembleBytes(nextByte(), nextByte());
  } else {
    disassembleByte(nextByte());
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

void disassembleMemoryModeNoDisp(int rm) {
  fprintf(verboseChannel, "decodeMemoryModeNoDisp -> rm: %02X\n", rm);

  int16_t disp = -1;
  if (rm == 0b110) {
    int byte0 = nextByte();
    int byte1 = nextByte();
    disp = (byte1 << 8) | byte0;
    fprintf(verboseChannel, "  -> disp: %02X\n", disp);
  }

  switch (rm) {
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

void disassembleRM(int byte1, int byte2) {
  int mod = (byte2 & MOV_MOD_MASK) >> 6;
  int w = byte1 & MOV_W_MASK;
  int rm = byte2 & MOV_RM_MASK;

  fprintf(verboseChannel, "decodeRM -> %02X %02X %02X\n", w, mod, rm);

  switch (mod) {
  case MEMORY_MODE_NO_DISP: {
    disassembleMemoryModeNoDisp(rm);
    break;
  }
  case MEMORY_MODE_8_BIT_DISP: {
    disassemblyMemoryMode8BitDisp(rm, nextByte());
    break;
  }
  case MEMORY_MODE_16_BIT_DISP: {
    disassembleMemoryMode16BitDisp(rm, nextByte(), nextByte());
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

int main(int argc, char *argv[]) {
  // last arg is always the filename
  source = fopen(argv[argc - 1], "rb");

  if (source == NULL) {
    perror("Error opening file");

    return 1;
  }

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-exec") == 0) {
      exec = 1;
    }

    if (strcmp(argv[i], "-v") == 0) {
      verbose = 1;
    }
  }

  if (verbose) {
    verboseChannel = stderr;
  } else {
    verboseChannel = fopen("/dev/null", "w");
  }

  printf("bits 16\n");

  int byte;
  while ((byte = nextByte()) != EOF) {
    int lo = (byte & 0b00001111);
    int hi = (byte & 0b11110000) >> 4;

    fprintf(verboseChannel, "considering -> %1X, %1X\n", hi, lo);

    opTable[hi][lo](byte);
  }

  fclose(source);
}
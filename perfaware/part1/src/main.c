#include <stdio.h>

#define BIT_D_MASK 0b00000010
#define BIT_W_MASK 0b00000001

#define MOV_W_MASK 0b00000001
#define MOV_D_MASK 0b00000010
#define MOV_REG_MASK 0b00111000
#define MOV_RM_MASK 0b00000111
#define MOV_MOD_MASK 0b11000000

#define MOV_IMM_W_MASK 0b00001000
#define MOV_IMM_REG_MASK 0b00000111

FILE *file;

#define nextByte() (fgetc(file))

void decodeREG(int w, int reg);
void decodeRM(int byte1, int byte2);
void decodeMemoryModeNoDisp(int rm);
void decodeByte(int byte);
void decodeBytes(int byte1, int byte2);

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

void NOOP(int byte0) {
  printf("NO_OP %02X\n", byte0);
  return;
}

// memory to accumulator / accumulator to memory
void MOVA(int byte0) {
  printf("mov ");

  int d = byte0 & BIT_D_MASK;
  int w = byte0 & BIT_W_MASK;

  fprintf(stderr, "MOVA d: %02X w: %02X\n", d, w);

  // logic for the d bit is inverted for
  // immediate to acc / acc to immediate
  // 1010000w - AX is dest
  // 1010001w - AX is source
  if (!d) {
    // always accumulator reg = 0
    decodeREG(w, 0);
    printf(", ");
    // kind of hacky, but we know we want [16-bit immediate]
    // and we know this does it
    decodeMemoryModeNoDisp(BP_);
  } else {
    // kind of hacky, but we know we want [16-bit immediate]
    // and we know this does it
    decodeMemoryModeNoDisp(BP_);
    printf(", ");
    // always accumulator reg = 0
    decodeREG(w, 0);
  }

  printf("\n");

  return;
}

// register/memory to/from register
void MOVR(int byte1) {
  int byte2 = nextByte();
  fprintf(stderr, "MOVR -> %02X %02X\n", byte1, byte2);
  printf("mov ");

  int w = byte1 & MOV_W_MASK;
  int reg = (byte2 & MOV_REG_MASK) >> 3;

  if (byte1 & MOV_D_MASK) {
    // reg is dest
    decodeREG(w, reg);
    printf(", ");
    decodeRM(byte1, byte2);
    printf("\n");
  } else {
    // reg is source
    decodeRM(byte1, byte2);
    printf(", ");
    decodeREG(w, reg);
    printf("\n");
  }
}

// immediate to register
void MOVI(int byte) {
  fprintf(stderr, "MOVI\n");
  printf("mov ");

  int w = (byte & MOV_IMM_W_MASK) >> 3;
  int reg = (byte & MOV_IMM_REG_MASK);

  decodeREG(w, reg);
  printf(", ");

  if (byte & MOV_IMM_W_MASK) {
    decodeBytes(nextByte(), nextByte());
  } else {
    decodeByte(nextByte());
  }

  printf("\n");
}

// immediate to register/memory
void MOVK(int byte1) {
  int byte2 = nextByte();
  // byte1 byte2 (DISP_LO) (DISP_HI) data (data if w = 1)
  fprintf(stderr, "MOVK -> %02X %02X\n", byte1, byte2);
  printf("mov ");

  decodeRM(byte1, byte2);
  printf(", ");

  if (byte1 & MOV_W_MASK) {
    printf("word ");
    decodeBytes(nextByte(), nextByte());
  } else {
    printf("byte ");
    decodeByte(nextByte());
  }

  printf("\n");
}

// clang-format off
void (*opTable[16][16])(int byte0) = {
    {NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP},
    {NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP},
    {NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP},
    {NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP},
    {NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP},
    {NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP},
    {NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP},
    {NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP},
    {NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, MOVR, MOVR, MOVR, MOVR, NOOP, NOOP, NOOP, NOOP},
    {NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP},
    {MOVA, MOVA, MOVA, MOVA, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP},
    {MOVI, MOVI, MOVI, MOVI, MOVI, MOVI, MOVI, MOVI, MOVI, MOVI, MOVI, MOVI, MOVI, MOVI, MOVI, MOVI},
    {NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, MOVK, MOVK, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP},
    {NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP},
    {NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP},
    {NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP},
};
// clang-format on

// @param w - 1 bit
// @param reg - 3 bits
void decodeREG(int w, int reg) {
  // fprintf(stderr, "decodeREG -> w: %02X, reg: %02X\n", w, reg);

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

void decodeMemoryModeNoDisp(int rm) {
  fprintf(stderr, "decodeMemoryModeNoDisp -> rm: %02X\n", rm);

  int16_t disp = -1;
  if (rm == 0b110) {
    int byte0 = nextByte();
    int byte1 = nextByte();
    disp = (byte1 << 8) | byte0;
    fprintf(stderr, "  -> disp: %02X\n", disp);
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

void decodeMemoryMode8BitDisp(int rm, int8_t byte) {
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

void decodeMemoryMode16BitDisp(int rm, int byte1, int byte2) {
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

void decodeRM(int byte1, int byte2) {
  int mod = (byte2 & MOV_MOD_MASK) >> 6;
  int w = byte1 & MOV_W_MASK;
  int rm = byte2 & MOV_RM_MASK;

  fprintf(stderr, "decodeRM -> %02X %02X %02X\n", w, mod, rm);

  switch (mod) {
  case MEMORY_MODE_NO_DISP: {
    decodeMemoryModeNoDisp(rm);
    break;
  }
  case MEMORY_MODE_8_BIT_DISP: {
    decodeMemoryMode8BitDisp(rm, nextByte());
    break;
  }
  case MEMORY_MODE_16_BIT_DISP: {
    decodeMemoryMode16BitDisp(rm, nextByte(), nextByte());
    break;
  }
  case REGISTER_MODE: {
    decodeREG(w, rm);
    break;
  }
  default:
    break;
  }
}

void decodeByte(int byte) { printf("%d", byte); }

void decodeBytes(int byte1, int byte2) { printf("%d", (byte2 << 8) | byte1); }

int main() {
  // read the file
  file = fopen("listing_0040_challenge_movs", "rb");
  // file = fopen("listing_0039_more_movs", "rb");

  if (file == NULL) {
    perror("Error opening file");

    return 1;
  }

  printf("bits 16\n");

  int byte;
  while ((byte = nextByte()) != EOF) {
    int lo = (byte & 0b00001111);
    int hi = (byte & 0b11110000) >> 4;

    fprintf(stderr, "considering -> %1X, %1X\n", hi, lo);

    opTable[hi][lo](byte);
  }

  fclose(file);
}
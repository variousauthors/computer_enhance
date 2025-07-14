#include <stdio.h>

#define MOV_W_MASK 0b00000001
#define MOV_D_MASK 0b00000010
#define MOV_REG_MASK 0b00111000
#define MOV_RM_MASK 0b00000111
#define MOV_MOD_MASK 0b11000000

#define MOV_IMM_W_MASK 0b00001000
#define MOV_IMM_REG_MASK 0b00000111

#define OPCODE_MASK 0b11111100

FILE *file;

#define nextByte() (fgetc(file))

enum OPCODE {
  MEM_TO_ACC = 0b10100000,
  IMM_TO_REGISTER = 0b10110000,
  MOV0 = 0b10001000,
  IMM_TO_REGISTER_MEMORY = 0b11000110,
  ACC_TO_MEM = 0b10100010,
};

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

void decodeOpImmediateToRegisterMemory(int byte1, int byte2) {
  // byte1 byte2 (DISP_LO) (DISP_HI) data (data if w = 1)
  fprintf(stderr, "decode imm to mem -> %02X %02X\n", byte1, byte2);
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

void decodeOpMemoryToAccumulator(int byte1) {
  printf("mov ");

  int w = byte1 & MOV_W_MASK;

  // always accumulator reg = 0
  decodeREG(w, 0);
  printf(", ");
  // kind of hacky, but we know we want [16-bit immediate]
  // and we know this does it
  decodeMemoryModeNoDisp(BP_);
  printf("\n");
}

void decodeOpAccumulatorToMemory(int byte1) {
  printf("mov ");

  int w = byte1 & MOV_W_MASK;

  // kind of hacky, but we know we want [16-bit immediate]
  // and we know this does it
  decodeMemoryModeNoDisp(BP_);
  printf(", ");
  // always accumulator reg = 0
  decodeREG(w, 0);
  printf("\n");
}

void decodeOpImmediateToRegister(int byte1, int byte2) {
  fprintf(stderr, "decode imm -> %02X %02X\n", byte1, byte2);
  printf("mov ");

  int w = (byte1 & MOV_IMM_W_MASK) >> 3;
  int reg = (byte1 & MOV_IMM_REG_MASK);

  decodeREG(w, reg);
  printf(", ");

  if (byte1 & MOV_IMM_W_MASK) {
    decodeBytes(byte2, nextByte());
  } else {
    decodeByte(byte2);
  }

  printf("\n");
}

void decodeOpRegisterMemoryToFromRegister(int byte1, int byte2) {
  fprintf(stderr, "decode reg/mem to reg -> %02X %02X\n", byte1, byte2);
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

int main() {
  // read the file
  file = fopen("listing_0040_challenge_movs", "rb");

  if (file == NULL) {
    perror("Error opening file");

    return 1;
  }

  printf("bits 16\n");

  int byte;
  while ((byte = nextByte()) != EOF) {
    fprintf(stderr, "considering -> %02X\n", byte);
    if ((byte & ACC_TO_MEM) == ACC_TO_MEM) {
      decodeOpAccumulatorToMemory(byte);
    } else if ((byte & MEM_TO_ACC) == MEM_TO_ACC) {
      decodeOpMemoryToAccumulator(byte);
    } else if ((byte & IMM_TO_REGISTER) == IMM_TO_REGISTER) {
      decodeOpImmediateToRegister(byte, nextByte());
    } else if ((byte & MOV0) == MOV0) {
      decodeOpRegisterMemoryToFromRegister(byte, nextByte());
    } else if ((byte & IMM_TO_REGISTER_MEMORY) == IMM_TO_REGISTER_MEMORY) {
      decodeOpImmediateToRegisterMemory(byte, nextByte());
    } else {
      fprintf(stderr, "unknown opcode 0x%02X\n", byte);
    }
  }

  fclose(file);
}
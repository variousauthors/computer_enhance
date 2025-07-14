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
  MOV0 = 0b10001000,
  IMMEDIATE_TO_REGISTER = 0b10110000,
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

void decodeRM(int byte1, int byte2) {
  int mod = byte2 & MOV_MOD_MASK;
  int w = byte1 & MOV_W_MASK;
  int reg = byte2 & MOV_RM_MASK;

  switch (mod) {
  case MOV_MOD_MASK: {
    decodeREG(w, reg);
  }
  /* code */
  break;

  default:
    break;
  }
}

void decodeByte(int byte) { printf("%d", byte); }

void decodeBytes(int byte1, int byte2) { printf("%d", (byte2 << 8) | byte1); }

void decodeOpImmediateToRegister(int byte1, int byte2) {
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
  printf("mov ");

  int w = byte1 & MOV_W_MASK;
  int reg = (byte2 & MOV_REG_MASK) >> 3;

  // fprintf(stderr, "OP -> w: %02X, reg: %02X\n", w, reg);

  // read D
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
  file = fopen("listing_0039_more_movs", "rb");

  if (file == NULL) {
    perror("Error opening file");

    return 1;
  }

  printf("bits 16\n");

  int byte;
  while ((byte = nextByte()) != EOF) {
    if ((byte & MOV0) == MOV0) {
      decodeOpRegisterMemoryToFromRegister(byte, fgetc(file));
    } else if ((byte & IMMEDIATE_TO_REGISTER) == IMMEDIATE_TO_REGISTER) {
      decodeOpImmediateToRegister(byte, nextByte());
    } else {
      fprintf(stderr, "unknown opcode 0x%02X\n", byte);
    }
  }

  fclose(file);
}
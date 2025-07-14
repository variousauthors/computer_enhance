#include <stdio.h>

#define MOV_W_MASK 0b00000001
#define MOV_D_MASK 0b00000010
#define MOV_REG_MASK 0b00111000
#define MOV_MOD_MASK 0b11000000
#define OPCODE_MASK 0b11111100

enum OPCODE {
  MOV0 = 0b10001000,
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

void decodeREG(int byte1, int byte2) {
  // code format 0000WREG
  int code = ((byte1 & MOV_W_MASK) << 3) | ((byte2 & MOV_REG_MASK) >> 3);

  switch (code) {
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

  switch (mod) {
  case MOV_MOD_MASK: {
    decodeREG(byte1, byte2 << 3);
  }
  /* code */
  break;

  default:
    break;
  }
}

void decodeOpRegisterMemoryToFromRegister(int byte1, int byte2) {
  printf("mov ");

  // read D
  if (byte1 & MOV_D_MASK) {
    // reg is dest
    decodeREG(byte1, byte2);
    printf(", ");
    decodeRM(byte1, byte2);
    printf("\n");
  } else {
    // reg is source
    decodeRM(byte1, byte2);
    printf(", ");
    decodeREG(byte1, byte2);
    printf("\n");
  }
}

int main() {
  // read the file
  FILE *file = fopen("listing_0038_many_register_mov", "rb");

  if (file == NULL) {
    perror("Error opening file");

    return 1;
  }

  int byte;
  while ((byte = fgetc(file)) != EOF) {
    switch (byte & OPCODE_MASK) {
    case MOV0: {
      decodeOpRegisterMemoryToFromRegister(byte, fgetc(file));
      break;
    }
    default:
      break;
    }
  }

  fclose(file);
}
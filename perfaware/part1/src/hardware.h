#ifndef HARDWARE_INC
#define HARDWARE_INC

#include <stdint.h>

#define BIT_D_MASK 0b00000010
#define BIT_W_MASK 0b00000001
#define REG_MASK 0b00111000
#define RM_MASK 0b00000111

#define IMED_SIGN_EXT_MASK BIT_D_MASK
#define IMED_CODE_MASK REG_MASK

#define MOV_W_MASK BIT_W_MASK
#define MOV_D_MASK BIT_D_MASK
#define MOV_REG_MASK REG_MASK
#define MOV_RM_MASK RM_MASK
#define MOV_MOD_MASK 0b11000000

#define MOV_IMM_W_MASK 0b00001000
#define MOV_IMM_REG_MASK 0b00000111

uint16_t REGISTERS[8];

enum MOD {
  MEMORY_MODE_NO_DISP, // DISP when R/M is 110
  MEMORY_MODE_8_BIT_DISP,
  MEMORY_MODE_16_BIT_DISP,
  REGISTER_MODE,
};

typedef enum REG_ENCODING {
  AL, // 0000
  CL, // 0001
  DL, // 0010
  BL, // 0011
  AH, // 0100
  CH, // 0101
  DH, // 0110
  BH, // 0111
  AX, // 1000
  CX, // 1001
  DX, // 1010
  BX, // 1011
  SP, // 1100
  BP, // 1101
  SI, // 1110
  DI, // 1111
} REG_ENCODING;

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

void registerStore8(int w, REG_ENCODING reg, uint8_t byte1);
void registerStore16(int w, REG_ENCODING reg, uint8_t byte1, uint8_t byte2);

void storeRegisterMemoryToFromRegister(int d, int w, int rm, int reg, int byte1,
                                       int byte2);

#endif
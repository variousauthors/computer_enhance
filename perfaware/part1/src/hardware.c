#include "hardware.h"
#include <stdint.h>

// 1 MB of memory!
uint8_t MEMORY[1024 * 1024] = {0};

/*                       AX CX DX BX SP BP SI DI */
uint16_t REGISTERS[8] = {0, 0, 0, 0, 0, 0, 0, 0};
char *REGISTER_NAMES[8] = {"AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI"};

Flags FLAGS = {0};

uint16_t IP = 0;

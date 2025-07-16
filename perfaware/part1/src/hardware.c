#include "hardware.h"
#include <stdint.h>

/*                       AX CX DX BC SP BP SI DI */
uint16_t REGISTERS[8] = {0, 0, 0, 0, 0, 0, 0, 0};
char *REGISTER_NAMES[8] = {"AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI"};

Flags FLAGS = {0};
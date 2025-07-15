
#include "decode.c"
#include "decode.h"
#include "disassemble.c"
#include "disassemble.h"
#include "global.c"
#include "global.h"
#include "hardware.c"
#include "hardware.h"
#include "optable.c"
#include "optable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

  if (exec) {
    printf("registers:\n");
    for (int i = 0; i < ArrayCount(REGISTERS); i++) {
      printf("%d: %02X\n", i, REGISTERS[i]);
    }
  }

  fclose(source);
}
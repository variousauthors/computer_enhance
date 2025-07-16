
#include "decode.c"
#include "decode.h"
#include "disassemble.c"
#include "disassemble.h"
#include "execute.c"
#include "execute.h"
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
  const char *filename = argv[argc - 1];

  FILE *file = fopen(filename, "rb");

  if (!file) {
    perror("Failed to open file");
    return 1;
  }

  // Seek to end to determine file size
  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  rewind(file);

  // Read file into buffer
  uint8_t bytesRead = fread(MEMORY, 1, size, file);
  if (bytesRead != size) {
    perror("Failed to read entire file");
    fclose(file);
    return 1;
  }

  // do the args
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

  // start the dissasembly
  initDisassembly();

  int byte;
  while (IP < bytesRead) {
    byte = nextByte();

    int lo = (byte & 0b00001111);
    int hi = (byte & 0b11110000) >> 4;

    fprintf(verboseChannel, "considering -> %1X, %1X\n", hi, lo);

    opTable[hi][lo](byte);
  }

  if (exec) {
    printf("registers:\n");
    printf("%s: 0x%04X\n", REGISTER_NAMES[0], REGISTERS[0]);
    printf("%s: 0x%04X\n", REGISTER_NAMES[3], REGISTERS[3]);
    printf("%s: 0x%04X\n", REGISTER_NAMES[1], REGISTERS[1]);
    printf("%s: 0x%04X\n", REGISTER_NAMES[2], REGISTERS[2]);
    printf("%s: 0x%04X\n", REGISTER_NAMES[4], REGISTERS[4]);
    printf("%s: 0x%04X\n", REGISTER_NAMES[5], REGISTERS[5]);
    printf("%s: 0x%04X\n", REGISTER_NAMES[6], REGISTERS[6]);
    printf("%s: 0x%04X\n", REGISTER_NAMES[7], REGISTERS[7]);

    printf("flags: %c%c\n", FLAGS.SF ? 'S' : ' ', FLAGS.ZF ? 'Z' : ' ');

    printf("ip: 0x%04X\n", IP);
  }
}
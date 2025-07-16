#ifndef GLOBAL_INC
#define GLOBAL_INC

#include <stdint.h>
#include <stdio.h>

uint8_t exec;
uint8_t verbose;

FILE *verboseChannel;
FILE *source;

#define ArrayCount(arr) (sizeof(arr) / sizeof((arr)[0]))

#endif
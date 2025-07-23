#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdint.h>
#include <stdio.h>

FILE *verboseChannel;
FILE *perfChannel;

int emit;
int verbose;
int perf;

typedef struct ProfilerTiming {
  char functionName[64];
  uint64_t begin;

} ProfilerTiming;

#define MAX_PROFILE_TIMERS 10

ProfilerTiming profileTimers[MAX_PROFILE_TIMERS];

void beginProfiler();
void endAndPrintProfiler();

#endif
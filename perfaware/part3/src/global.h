#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdint.h>
#include <stdio.h>

FILE *verboseChannel;
FILE *perfChannel;

int emit;
int verbose;
int perf;

typedef struct ProfilerTimer {
  char label[19];
  int active;
  int hits;
  unsigned long h;
  uint64_t begin;
  uint64_t initTime;
  uint64_t processedBytesCount;

  uint64_t total;
  uint64_t exclusive;

} ProfilerTimer;

#define ArrayCount(arr) (sizeof(arr) / sizeof((arr)[0]))

#define PROFILER

#ifndef PROFILER
#define TimeFunction
#define TimeBlock(...)
#define TimeBandwidth(...)
#else

#define TimeBandwidth(bytes) TimeBlock(__func__, bytes)

#define TimeBlock(name, bytes)                                                 \
  __attribute__((cleanup(stop))) unsigned long parentTimer = start(name, bytes)
#endif

#define TimeFunction TimeBlock(__func__, 0)

#define MAX_PROFILE_TIMERS 4096

ProfilerTimer profileTimers[MAX_PROFILE_TIMERS];

unsigned long start(const char *name, uint64_t bytesProcessed);
void stop(unsigned long *parentHash);
void stopProfilerTimer(const char *name, const char *id);
void startProfilerTimer(const char *name, const char *id);
void beginProfiler();
void endAndPrintProfiler();

#endif
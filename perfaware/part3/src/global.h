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

  uint64_t total;
  uint64_t exclusive;

} ProfilerTimer;

#define PROFILER

#ifndef PROFILER
#define ProfilerMagic
#else
// declares a variable with __attribute__(cleanup) so that
// it will call a destructor when it goes out of scope
#define ProfilerMagic                                                          \
  __attribute__((cleanup(stop))) unsigned long parentTimer = start(__func__)
#endif

#define MAX_PROFILE_TIMERS 4096

ProfilerTimer profileTimers[MAX_PROFILE_TIMERS];

unsigned long start(const char *name);
void stop(unsigned long *parentHash);
void stopProfilerTimer(const char *name, const char *id);
void startProfilerTimer(const char *name, const char *id);
void beginProfiler();
void endAndPrintProfiler();

#endif
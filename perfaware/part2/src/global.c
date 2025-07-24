#include "global.h"
#include "os_metrics.h"
#include <stdint.h>
#include <string.h>

FILE *verboseChannel = 0;
FILE *perfChannel = 0;
ProfilerTimer profileTimers[MAX_PROFILE_TIMERS] = {0};
int currentProfileTimer = 0;

uint64_t cpuFreq = 0;

/**
 * OK so yeah OK so.
 *
 * when we enter a function
 * we set the global "current function"
 * and when we exit, we restore the "current function"
 * and subtract the time from the elapsedNoChildren
 *
 */

unsigned long hash(const char *str1, const char *str2) {
  unsigned long hash = 5381;
  int c;

  while ((c = (unsigned char)*str1++))
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

  while ((c = (unsigned char)*str2++))
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

  return hash & 0xFFF;
}

ProfilerTimer *currentTimer = &profileTimers[0];

unsigned long start(const char *name) {
  uint64_t now = ReadCPUTimer();

  unsigned long parentHash = currentTimer->h;
  unsigned long h = hash(name, "1");

  ProfilerTimer *timer = &profileTimers[h];
  strcpy(timer->label, name);

  timer->active = 1;
  timer->hits = 0;
  timer->h = h;
  uint64_t begin = now;
  timer->begin = begin;

  // fprintf(stderr, "start: %s, parent: %ld, h: %ld\n", timer->label,
  // parentHash,
  //         h);

  /* when we start a new timer, we have to pause the parent */
  uint64_t elapsed = (now - currentTimer->begin);
  currentTimer->elapsed += elapsed;
  currentTimer->accumulator += elapsed;
  currentTimer->elapsedNoChildren += elapsed;

  // fprintf(stderr, "  start: adding to %s: %lld\n", currentTimer->label,
  //         elapsed);

  currentTimer = timer;

  // pack the two hashes together
  return (parentHash << 12) | h;
}

void stop(unsigned long *hashes) {
  uint64_t now = ReadCPUTimer();
  unsigned long parentHash = *hashes >> 12;
  unsigned long h = *hashes & 0xFFF;

  ProfilerTimer *timer = &profileTimers[h];
  ProfilerTimer *parentTimer = &profileTimers[parentHash];

  // fprintf(stderr, "stop: %s, parent: %ld, h: %ld\n", timer->label,
  // parentHash,
  //         h);

  if (timer->label[0] == 0) {
    fprintf(perfChannel, "tried to update a timer that did not exist\n");
    return;
  }

  timer->active = 0;
  uint64_t end = now;
  uint64_t elapsed = (end - timer->begin);
  timer->accumulator += elapsed;
  timer->elapsed += elapsed;
  timer->elapsedNoChildren += elapsed;

  // fprintf(stderr, "  stop: adding to %s: %lld\n", timer->label, elapsed);

  parentTimer->begin = now;

  if (parentTimer != timer) {
    // it adds in the time from this (child) timer
    parentTimer->elapsed += timer->accumulator;
    parentTimer->accumulator += timer->accumulator;
    // fprintf(stderr, "  stop: adding to %s: %lld\n", parentTimer->label,
    //         timer->accumulator);
    timer->accumulator = 0;
  }

  currentTimer = parentTimer;
}

void beginProfiler() {
  if (!perf) {
    return;
  }

  ProfilerTimer *timer = &profileTimers[0];

  strcpy(timer->label, "main");
  timer->active = 1;
  timer->hits = 0;
  timer->h = 0;
  uint64_t begin = ReadCPUTimer();
  timer->begin = begin;

  cpuFreq = EstimateCPUTimerFreq();
}

void endAndPrintProfiler() {
  ProfilerTimer *timer = &profileTimers[0];

  fprintf(perfChannel, "\nprofiler timers:\n");

  uint64_t totalElapsed = timer->elapsed;

  fprintf(perfChannel, "total ticks: %lld", totalElapsed);

  if (cpuFreq) {
    fprintf(perfChannel, "  Total time: %0.4fms (CPU freq %llu)\n\n",
            1000.0 * (double)totalElapsed / (double)cpuFreq, cpuFreq);
  }

  int count = 0;
  for (int i = 0; i < MAX_PROFILE_TIMERS; i++) {
    ProfilerTimer timer = profileTimers[i];

    if (timer.active > 0) {
      fprintf(perfChannel, "forgot to end timer: %s\n", timer.label);
    }

    if (timer.elapsed > 0) {
      fprintf(perfChannel, "  %d. ", count++);
      PrintTimeElapsed(timer.label, totalElapsed, timer.elapsed,
                       timer.elapsedNoChildren);
    }
  }
}

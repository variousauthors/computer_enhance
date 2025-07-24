#include "global.h"
#include "os_metrics.h"
#include <string.h>

FILE *verboseChannel = 0;
FILE *perfChannel = 0;
ProfilerTimer profileTimers[MAX_PROFILE_TIMERS] = {0};
int currentProfileTimer = 0;

uint64_t profilerStart = 0;
uint64_t profilerEnd = 0;
uint64_t totalElapsed = 0;
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

ProfilerTimer *currentTimer;

void startProfilerTimer(const char *name, const char *id) {
  unsigned long h = hash(name, id);
  ProfilerTimer *timer = &profileTimers[h];

  if (currentTimer) {
    /* when we start a new timer, we have to pause the parent */
    uint64_t elapsed = (ReadCPUTimer() - currentTimer->begin);
    currentTimer->elapsed += elapsed;
    currentTimer->elapsedNoChildren += elapsed;
  }

  // however, if the parent identity has not changed
  // we don't overwrite, ie if we have A -> B -> B
  // we don't want to lose track of A
  if (currentTimer != timer) {
    timer->parent = currentTimer;
    currentTimer = timer;
  }

  strcpy(timer->label, name);

  timer->active = 1;
  timer->hits = 0;
  uint64_t begin = ReadCPUTimer();
  timer->begin = begin;
}

void stopProfilerTimer(const char *name, const char *id) {
  unsigned long h = hash(name, id);
  ProfilerTimer *timer = &profileTimers[h];

  if (timer->label[0] == 0) {
    fprintf(perfChannel, "tried to update a timer that did not exist\n");
    return;
  }

  timer->active = 0;
  uint64_t end = ReadCPUTimer();
  uint64_t elapsed = (end - timer->begin);
  timer->elapsed += elapsed;
  timer->elapsedNoChildren += elapsed;

  if (timer->parent) {
    // we have to unpause the parent timer
    currentTimer = timer->parent;
    // it starts counting it's own no children time
    currentTimer->begin = ReadCPUTimer();

    if (timer->parent != timer) {
      // it adds in the time from this (child) timer
      currentTimer->elapsed += elapsed;
    }
  }
}

void beginProfiler() {
  if (!perf) {
    return;
  }

  profilerStart = ReadCPUTimer();
  fprintf(perfChannel, "starting: %lld\n", profilerStart);
  cpuFreq = EstimateCPUTimerFreq();
}

void endAndPrintProfiler() {
  profilerEnd = ReadCPUTimer();
  fprintf(perfChannel, "ending: %lld\n", profilerEnd);

  fprintf(perfChannel, "\nprofiler timers:\n");

  totalElapsed = profilerEnd - profilerStart;

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

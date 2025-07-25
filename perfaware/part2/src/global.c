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
  ProfilerTimer *parentTimer = &profileTimers[parentHash];

  // stop the parent timer
  parentTimer->exclusive += now - parentTimer->begin;
  parentTimer->active--;

  // init this timer if we need to
  if (timer->active == 0) {
    strcpy(timer->label, name);
    timer->h = h;
    timer->initTime = now;
  }

  // start this timer
  timer->active++;
  timer->hits++;
  timer->begin = now;

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

  if (timer->label[0] == 0) {
    fprintf(perfChannel, "tried to update a timer that did not exist\n");
    return;
  }

  // stop this timer
  timer->active--;
  timer->exclusive += now - timer->begin;

  if (timer->active == 0) {
    // record total
    timer->total += now - timer->initTime;
  }

  // start the parent
  parentTimer->active++;
  parentTimer->begin = now;

  currentTimer = parentTimer;
}

uint64_t profilerStartTime;

void beginProfiler() {
  uint64_t now = ReadCPUTimer();
  profilerStartTime = now;

  if (!perf) {
    return;
  }

  ProfilerTimer *timer = &profileTimers[0];

  strcpy(timer->label, "main");
  timer->active++;
  timer->hits++;
  timer->h = 0;
  timer->begin = now;
  timer->initTime = now;

  cpuFreq = EstimateCPUTimerFreq();
  currentTimer = timer;
}

void endAndPrintProfiler() {
  uint64_t now = ReadCPUTimer();
  ProfilerTimer *timer = &profileTimers[0];

  // stop the root timer
  timer->exclusive += now - timer->begin;
  timer->active--;
  timer->total = now - timer->initTime;

  uint64_t totalElapsed = now - profilerStartTime;

  if (cpuFreq) {
    fprintf(perfChannel, "\nTotal time: %0.4fms (CPU freq %llu)\n\n",
            1000.0 * (double)totalElapsed / (double)cpuFreq, cpuFreq);
  }

#ifdef PROFILER
  int count = 0;
  for (int i = 0; i < MAX_PROFILE_TIMERS; i++) {
    ProfilerTimer timer = profileTimers[i];

    if (timer.active > 0) {
      fprintf(perfChannel, "forgot to end timer: %s\n", timer.label);
    }

    if (timer.exclusive > 0) {
      fprintf(perfChannel, "  %d. ", count++);
      PrintTimeElapsed(timer.label, totalElapsed, timer);
    }
  }
#endif
}

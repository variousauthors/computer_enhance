#include "global.h"
#include "os_metrics.h"

FILE *verboseChannel = 0;
FILE *perfChannel = 0;
ProfilerTiming profileTimers[MAX_PROFILE_TIMERS] = {0};
int currentProfileTimer = 0;

uint64_t profilerStart = 0;
uint64_t profilerEnd = 0;
uint64_t totalElapsed = 0;
uint64_t cpuFreq = 0;

void beginProfiler() {
  if (!perf) {
    return;
  }

  profilerStart = ReadCPUTimer();
  cpuFreq = EstimateCPUTimerFreq();

  fprintf(perfChannel, "starting profiler with freq: %lld", cpuFreq);
}

void endAndPrintProfiler() {
  profilerEnd = ReadCPUTimer();

  fprintf(perfChannel, "\nprofiler timings:\n");

  totalElapsed = profilerEnd - profilerStart;

  if (cpuFreq) {
    fprintf(perfChannel, "  Total time: %0.4fms (CPU freq %llu)\n\n",
            1000.0 * (double)totalElapsed / (double)cpuFreq, cpuFreq);
  }

  for (int i = 0; i < MAX_PROFILE_TIMERS; i++) {
    ProfilerTiming timer = profileTimers[i];

    if (timer.begin > 0) {
      uint64_t begin = timer.begin;
      uint64_t end = (i + 1) < MAX_PROFILE_TIMERS
                         ? profileTimers[i + 1].begin > 0
                               ? profileTimers[i + 1].begin
                               : profilerEnd
                         : profilerEnd;

      fprintf(perfChannel, "%d. ", i);
      PrintTimeElapsed(timer.functionName, totalElapsed, begin, end);
    }
  }
}

#define TimeFunction                                                           \
  do {                                                                         \
    profileTimers[currentProfileTimer].begin = ReadCPUTimer();                 \
    strcpy(profileTimers[currentProfileTimer++].functionName, __func__);       \
  } while (0)

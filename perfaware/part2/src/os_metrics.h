#ifndef OS_METRICS_H
#define OS_METRICS_H

#include <stdint.h>

static uint64_t ReadOSTimer(void);
static inline uint64_t ReadCPUTimer(void);
static uint64_t GetOSTimerFreq(void);
static void PrintTimeElapsed(char const *Label, uint64_t TotalTSCElapsed,
                             uint64_t Begin, uint64_t End);
static uint64_t EstimateCPUTimerFreq(void);

#endif
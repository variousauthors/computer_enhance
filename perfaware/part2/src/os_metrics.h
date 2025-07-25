#ifndef OS_METRICS_H
#define OS_METRICS_H

#include "global.h"
#include <stdint.h>

uint64_t ReadOSTimer(void);
uint64_t ReadCPUTimer(void);
uint64_t GetOSTimerFreq(void);
void PrintTimeElapsed(char const *Label, uint64_t TotalTSCElapsed,
                      ProfilerTimer timer);
uint64_t EstimateCPUTimerFreq(void);

#endif
#ifndef OS_METRICS_H
#define OS_METRICS_H

#include <stdint.h>

uint64_t ReadOSTimer(void);
uint64_t ReadCPUTimer(void);
uint64_t GetOSTimerFreq(void);
void PrintTimeElapsed(char const *Label, uint64_t TotalTSCElapsed,
                      uint64_t Begin, uint64_t End);
uint64_t EstimateCPUTimerFreq(void);

#endif
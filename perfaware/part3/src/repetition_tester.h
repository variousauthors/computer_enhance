#ifndef REPETITION_TESTER_H
#define REPETITION_TESTER_H

#include <stdint.h>

typedef struct ResultRepetitionTester {
  uint64_t min;
  uint64_t max;
  uint64_t average;

} ResultRepetitionTester;

typedef struct StateRepetitionTester {
  uint64_t timer;
  uint64_t timerMax; // the longest we plan to wait between updating the min
  uint64_t runs;     // number of runs
  uint64_t pageFaults; // number of page faults in the run (all runs will be the same)

  uint64_t cpuFreq;
  uint64_t bytesPerRun; // the number of bytes processed per run

} StateRepetitionTester;

typedef struct SubectUnderTestRepetitionTester {
  char *label;
  void (*func)();
  void (*setup)();

  ResultRepetitionTester *result;
  StateRepetitionTester *state;

} SubectUnderTestRepetitionTester;


#endif
#ifndef REPETITION_TESTER_H
#define REPETITION_TESTER_H

#include <stdint.h>

typedef struct ResultRepetitionTester {
  uint64_t time;
  uint64_t pageFaults;
  uint64_t bytes;

} ResultRepetitionTester;

typedef struct StateRepetitionTester {
  uint64_t timer;
  uint64_t timerMax; // the longest we plan to wait between updating the min
  uint64_t runs;     // number of runs

  uint64_t cpuFreq;
  uint64_t bytesPerRun; // the number of bytes processed per run

} StateRepetitionTester;

typedef struct SubectUnderTestRepetitionTester {
  char *label;
  void (*func)();
  void (*setup)();
  void (*teardown)();

  StateRepetitionTester *state;

  ResultRepetitionTester *min;
  ResultRepetitionTester *max;
  ResultRepetitionTester *average;

} SubectUnderTestRepetitionTester;

SubectUnderTestRepetitionTester *EMPTY_SUBJECT_UNDER_TEST_REPETITION_TESTER;

#endif
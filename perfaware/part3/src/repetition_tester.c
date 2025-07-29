#include <stdint.h>

#include "global.h"
#include "os_metrics.h"
#include "repetition_tester.h"

#include <stdlib.h>
#include <sys/mman.h>

double MEGABYTE = 1024 * 1024;
double GIGABYTE = 1024 * 1024 * 1024;

void printSingleTiming(char *label, double t, uint64_t cpuFreq,
                       double bytesPerRun, uint64_t pageFaults) {
  double minMs = 1000.0 * t / (double)cpuFreq;
  int mbPerRun = bytesPerRun / MEGABYTE;
  double gbPerRun = bytesPerRun / GIGABYTE;

  fprintf(
      stderr,
      "%s: %10.4fms (CPU freq %llu) (%dmb processed at %fgb/s) (PF: %lld)\n",
      label, minMs, cpuFreq, mbPerRun, gbPerRun / minMs * 1000, pageFaults);
}

void printResultRepetitionTester(SubectUnderTestRepetitionTester *subject) {
  ResultRepetitionTester result = *subject->result;
  StateRepetitionTester state = *subject->state;

  uint64_t megabyte = 1024 * 1024;
  uint64_t gigabyte = megabyte * 1024;
  double minMs = 1000.0 * (double)result.min / (double)state.cpuFreq;
  double mbPerRun = (double)state.bytesPerRun / megabyte;
  double gbPerRun = (double)state.bytesPerRun / gigabyte;

  fprintf(stderr, "%s:\n", subject->label);
  printSingleTiming("  min", result.min, state.cpuFreq, state.bytesPerRun,
                    state.pageFaults);
  printSingleTiming("  max", result.max, state.cpuFreq, state.bytesPerRun,
                    state.pageFaults);
  printSingleTiming("  avg", result.average, state.cpuFreq, state.bytesPerRun,
                    state.pageFaults);
}

void initResultRepetitionTester(ResultRepetitionTester *result) {
  result->max = 0;
  result->min = UINT64_MAX;
  result->average = 0;
}

void initStateRepetitionTester(StateRepetitionTester *state,
                               uint64_t bytesPerRun, uint64_t maxTimeSeconds) {
  state->runs = 0;
  state->timer = 0;
  state->pageFaults = 0;
  state->cpuFreq = EstimateCPUTimerFreq();
  state->timerMax = state->cpuFreq * maxTimeSeconds;
  state->bytesPerRun = bytesPerRun;
}

SubectUnderTestRepetitionTester *
initSubectUnderTestRepetitionTester(char *label, void (*func)(),
                                    void (*setup)(), uint64_t bytesPerRun,
                                    uint64_t maxTimeSeconds) {
  SubectUnderTestRepetitionTester *subject =
      malloc(sizeof(SubectUnderTestRepetitionTester));
  ResultRepetitionTester *result = malloc(sizeof(ResultRepetitionTester));
  StateRepetitionTester *state = malloc(sizeof(StateRepetitionTester));

  initResultRepetitionTester(result);
  initStateRepetitionTester(state, bytesPerRun, maxTimeSeconds);

  subject->func = func;
  subject->setup = setup;
  subject->state = state;
  subject->result = result;
  subject->label = label;

  return subject;
}

SubectUnderTestRepetitionTester *subjects[2];

void printResultsRepetitiontester() {
  for (int i = 0; i < ArrayCount(subjects); i++) {
    printResultRepetitionTester(subjects[i]);
  }
}

void runRepetitionTester() {
  for (int i = 0; i < ArrayCount(subjects); i++) {
    SubectUnderTestRepetitionTester *subject = subjects[i];
    StateRepetitionTester *state = subject->state;
    ResultRepetitionTester *result = subject->result;

    subject->setup();
    int initialRun = 1;

    for (;;) {
      char *buffer;
      uint64_t initialPF = ReadOSPageFaultCount();
      uint64_t start = ReadCPUTimer();
      subject->func();
      uint64_t t = ReadCPUTimer() - start;
      uint64_t pf = ReadOSPageFaultCount() - initialPF;

      if (initialRun) {
        state->pageFaults = pf;
        initialRun = 0;
      }

      if (t < result->min) {
        result->min = t;
        state->timer = 0; // reset timer every time we find a new min
      }

      if (t > result->max) {
        result->max = t;
      }

      // calculate the next average
      result->average =
          ((result->average * state->runs) + t) / ((state->runs) + 1);
      state->timer += t;
      state->runs++;

      if (state->timer >= state->timerMax) {
        break;
      }
    }
  }
}

/** tests */

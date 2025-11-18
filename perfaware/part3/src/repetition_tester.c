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
  double pfPerKb = bytesPerRun / pageFaults;

  if (pageFaults) {
    fprintf(stderr,
            "%s: %10.4fms (CPU freq %llu) (%d mb processed at %0.2f gb/s) (PF: "
            "%lld,  %0.2f bytes/pf)\n",
            label, minMs, cpuFreq, mbPerRun, gbPerRun / minMs * 1000,
            pageFaults, pfPerKb);
  } else {
    fprintf(stderr,
            "%s: %10.4fms (CPU freq %llu) (%d mb processed at %0.2f gb/s)\n",
            label, minMs, cpuFreq, mbPerRun, gbPerRun / minMs * 1000);
  }
}

void printResultRepetitionTester(SubectUnderTestRepetitionTester *subject) {
  ResultRepetitionTester min = *subject->min;
  ResultRepetitionTester max = *subject->max;
  ResultRepetitionTester avg = *subject->average;
  StateRepetitionTester state = *subject->state;

  fprintf(stderr, "\n--- %s ---\n", subject->label);
  printSingleTiming("  min", min.time, state.cpuFreq, min.bytes,
                    min.pageFaults);
  printSingleTiming("  max", max.time, state.cpuFreq, max.bytes,
                    max.pageFaults);
  printSingleTiming("  avg", avg.time, state.cpuFreq, avg.bytes,
                    avg.pageFaults);
}

void initResultRepetitionTester(ResultRepetitionTester *result) {
  result->time = 0;
  result->pageFaults = 0;
  result->bytes = 0;
}

void initStateRepetitionTester(StateRepetitionTester *state,
                               uint64_t bytesPerRun, uint64_t maxTimeSeconds) {
  state->runs = 0;
  state->timer = 0;
  state->cpuFreq = EstimateCPUTimerFreq();
  state->timerMax = state->cpuFreq * maxTimeSeconds;
  state->bytesPerRun = bytesPerRun;
}

SubectUnderTestRepetitionTester *initSubectUnderTestRepetitionTester(
    char *label, void (*func)(), void (*setup)(), void(*teardown),
    uint64_t bytesPerRun, uint64_t maxTimeSeconds) {
  SubectUnderTestRepetitionTester *subject =
      malloc(sizeof(SubectUnderTestRepetitionTester));
  ResultRepetitionTester *min = malloc(sizeof(ResultRepetitionTester));
  ResultRepetitionTester *max = malloc(sizeof(ResultRepetitionTester));
  ResultRepetitionTester *average = malloc(sizeof(ResultRepetitionTester));
  StateRepetitionTester *state = malloc(sizeof(StateRepetitionTester));

  initResultRepetitionTester(min);
  initResultRepetitionTester(max);
  initResultRepetitionTester(average);
  initStateRepetitionTester(state, bytesPerRun, maxTimeSeconds);

  subject->func = func;
  subject->setup = setup;
  subject->teardown = teardown;
  subject->state = state;
  subject->min = min;
  subject->min->time = UINT64_MAX;
  subject->max = max;
  subject->average = average;
  subject->label = label;

  return subject;
}

SubectUnderTestRepetitionTester *EMPTY_SUBJECT_UNDER_TEST_REPETITION_TESTER = 0;

SubectUnderTestRepetitionTester *subjects[4];

void printResultsRepetitiontester() {
  for (int i = 0; i < ArrayCount(subjects); i++) {
    if (subjects[i] == EMPTY_SUBJECT_UNDER_TEST_REPETITION_TESTER) {
      continue;
    }

    printResultRepetitionTester(subjects[i]);
  }
}

void runRepetitionTester() {
  for (int i = 0; i < ArrayCount(subjects); i++) {
    SubectUnderTestRepetitionTester *subject = subjects[i];

    if (subject == EMPTY_SUBJECT_UNDER_TEST_REPETITION_TESTER) {
      continue;
    }

    StateRepetitionTester *state = subject->state;

    int initialRun = 1;
    subject->setup();

    for (;;) {

      uint64_t initialPF = ReadOSPageFaultCount();
      uint64_t start = ReadCPUTimer();
      subject->func();
      uint64_t t = ReadCPUTimer() - start;
      uint64_t pf = ReadOSPageFaultCount() - initialPF;

      subject->teardown();

      if (t < subject->min->time) {
        subject->min->time = t;
        subject->min->pageFaults = pf;
        subject->min->bytes = state->bytesPerRun;
        state->timer = 0; // reset timer every time we find a new min
      }

      if (t > subject->max->time) {
        subject->max->time = t;
        subject->max->pageFaults = pf;
        subject->max->bytes = state->bytesPerRun;
      }

      // calculate the next average
      subject->average->time =
          ((subject->average->time * state->runs) + t) / ((state->runs) + 1);
      subject->average->pageFaults =
          ((subject->average->pageFaults * state->runs) + pf) /
          ((state->runs) + 1);
      subject->average->bytes =
          ((subject->average->bytes * state->runs) + state->bytesPerRun) /
          ((state->runs) + 1);

      state->timer += t;
      state->runs++;

      if (state->timer >= state->timerMax) {
        break;
      }
    }
  }
}

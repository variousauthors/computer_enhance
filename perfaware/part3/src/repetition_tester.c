#include <stdint.h>

#include "global.c"
#include "global.h"
#include "os_metrics.c"
#include "os_metrics.h"

/* given a function we run that function over and over
 * recording the run-time and looking for the min, max and average */

typedef struct ResultRepetitionTester {
  uint64_t min;
  uint64_t max;
  uint64_t average;

} ResultRepetitionTester;

/** the tester "searches" for a min by running for timer ms
 * and reseting the timer whenevr it hits a new min
 */
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

  ResultRepetitionTester *result;
  StateRepetitionTester *state;

} SubectUnderTestRepetitionTester;

double MEGABYTE = 1024 * 1024;
double GIGABYTE = 1024 * 1024 * 1024;

void printSingleTiming(char *label, double t, uint64_t cpuFreq,
                       double bytesPerRun) {
  double minMs = 1000.0 * t / (double)cpuFreq;
  int mbPerRun = bytesPerRun / MEGABYTE;
  double gbPerRun = bytesPerRun / GIGABYTE;

  fprintf(stderr, "%s: %10.4fms (CPU freq %llu) (%dmb processed at %fgb/s)\n",
          label, minMs, cpuFreq, mbPerRun, gbPerRun / minMs * 1000);
}

void printResultRepetitionTester(SubectUnderTestRepetitionTester *subject) {
  ResultRepetitionTester result = *subject->result;
  StateRepetitionTester state = *subject->state;

  uint64_t megabyte = 1024 * 1024;
  uint64_t gigabyte = megabyte * 1024;
  double minMs = 1000.0 * (double)result.min / (double)cpuFreq;
  double mbPerRun = (double)state.bytesPerRun / megabyte;
  double gbPerRun = (double)state.bytesPerRun / gigabyte;

  fprintf(stderr, "%s:\n", subject->label);
  printSingleTiming("  min", result.min, state.cpuFreq, state.bytesPerRun);
  printSingleTiming("  max", result.max, state.cpuFreq, state.bytesPerRun);
  printSingleTiming("  avg", result.average, state.cpuFreq, state.bytesPerRun);
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

    for (;;) {
      char *buffer;
      uint64_t start = ReadCPUTimer();
      subject->func();
      uint64_t t = ReadCPUTimer() - start;

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

#define FILE_SIZE 785560000

char *buffer;
void readFileNoMallocSetup() { buffer = malloc(FILE_SIZE); }
void readFileNoMalloc() { read_file_no_alloc("in.json", buffer); }

void readFile() { char *buffer = read_file("in.json"); }

void NO_SETUP() {}

int main(int arc, char **argv) {
  int maxTime = 10;
  maxTime = atoi(argv[arc - 1]);

  SubectUnderTestRepetitionTester *readFileTest =
      initSubectUnderTestRepetitionTester("readFile", readFile, NO_SETUP,
                                          FILE_SIZE, maxTime);

  SubectUnderTestRepetitionTester *readFileNoMallocTest =
      initSubectUnderTestRepetitionTester("readFileNoMalloc", readFileNoMalloc,
                                          readFileNoMallocSetup, FILE_SIZE,
                                          maxTime);

  subjects[0] = readFileNoMallocTest;
  subjects[1] = readFileTest;

  runRepetitionTester();

  printResultsRepetitiontester();

  return 0;
}
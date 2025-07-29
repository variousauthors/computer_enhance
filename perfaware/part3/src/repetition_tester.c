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

void printResultRepetitionTester(StateRepetitionTester state,
                                 ResultRepetitionTester result) {

  uint64_t megabyte = 1024 * 1024;
  uint64_t gigabyte = megabyte * 1024;
  double minMs = 1000.0 * (double)result.min / (double)cpuFreq;
  double mbPerRun = (double)state.bytesPerRun / megabyte;
  double gbPerRun = (double)state.bytesPerRun / gigabyte;

  printSingleTiming("min", result.min, state.cpuFreq, state.bytesPerRun);
  printSingleTiming("max", result.max, state.cpuFreq, state.bytesPerRun);
  printSingleTiming("avg", result.average, state.cpuFreq, state.bytesPerRun);
}

void initResultRepetitionTester(ResultRepetitionTester *result) {
  result->max = 0;
  result->min = UINT64_MAX;
  result->average = 0;
}

#define MAX_TIME_SEC 100;

void initStateRepetitionTester(StateRepetitionTester *state,
                               uint64_t bytesPerRun) {
  state->runs = 0;
  state->timer = 0;
  state->cpuFreq = EstimateCPUTimerFreq();
  state->timerMax = state->cpuFreq * MAX_TIME_SEC;
  state->bytesPerRun = bytesPerRun;
}

ResultRepetitionTester result;
StateRepetitionTester state;

void handle_sigint(int sig) {
  fprintf(stderr, "\n");
  printResultRepetitionTester(state, result);
  exit(0); // Clean exit
}

int main() {
  signal(SIGINT, handle_sigint);

  initResultRepetitionTester(&result);
  initStateRepetitionTester(&state, 749 * 1024 * 1024);

  for (;;) {
    char *buffer;
    uint64_t start = ReadCPUTimer();
    buffer = read_file("in.json");
    uint64_t t = ReadCPUTimer() - start;

    if (t < result.min) {
      result.min = t;
      state.timer = 0; // reset timer every time we find a new min
    }

    if (t > result.max) {
      result.max = t;
    }

    // calculate the next average
    result.average = ((result.average * state.runs) + t) / ((state.runs) + 1);
    state.timer += t;
    state.runs++;

    if (state.timer >= state.timerMax) {
      break;
    }
  }

  return 0;
}
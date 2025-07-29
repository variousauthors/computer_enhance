
#include "global.c"
#include "global.h"
#include "os_metrics.c"
#include "os_metrics.h"
#include "repetition_tester.c"
#include "repetition_tester.h"

#include <sys/mman.h>

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
      initSubectUnderTestRepetitionTester("malloc + readFile", readFile,
                                          NO_SETUP, FILE_SIZE, maxTime);

  SubectUnderTestRepetitionTester *readFileNoMallocTest =
      initSubectUnderTestRepetitionTester("readFile", readFileNoMalloc,
                                          readFileNoMallocSetup, FILE_SIZE,
                                          maxTime);

  subjects[0] = readFileTest;
  subjects[1] = readFileNoMallocTest;

  runRepetitionTester();

  printResultsRepetitiontester();

  return 0;
}
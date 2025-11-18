
#include "global.c"
#include "global.h"
#include "os_metrics.c"
#include "os_metrics.h"
#include "repetition_tester.c"
#include "repetition_tester.h"

#include <sys/mman.h>

extern int read_x1(int *xs, int count) __attribute__((used));

#define FILE_SIZE 785560000

void printBits(uint64_t bits) {
  int offset = 14;
  int L3 = 9;
  int L2 = 9;

  for (int i = 0; i < (64 - offset - L3 - L2); i++) {
    uint8_t bit = (bits >> 63);
    fprintf(stdout, "%d", bit);
    bits <<= 1;
  }

  fprintf(stdout, " | ");

  for (int i = 0; i < L2; i++) {
    uint8_t bit = (bits >> 63);
    fprintf(stdout, "%d", bit);
    bits <<= 1;
  }

  fprintf(stdout, " | ");

  for (int i = 0; i < L3; i++) {
    uint8_t bit = (bits >> 63);
    fprintf(stdout, "%d", bit);
    bits <<= 1;
  }

  fprintf(stdout, " | ");

  for (int i = 0; i < offset; i++) {
    uint8_t bit = (bits >> 63);
    fprintf(stdout, "%d", bit);
    bits <<= 1;
  }

  fprintf(stdout, "\n");
}

void printPointerDecomposition(void *ptr) {
  uint64_t address = (uint64_t)ptr;

  printBits(address);

  uint16_t offset = (address) & 0b11111111111111;
  uint16_t L3 = (address >> (0 * 9 + 14)) & 0b111111111;
  uint16_t L2 = (address >> (1 * 9 + 14)) & 0b111111111;
  uint16_t L1 = (address >> (2 * 9 + 14)) & 0b111111111;
  uint16_t L0 = (address >> (2 * 9 + 2 + 14)) & 0b11;

  // fprintf(stdout, "%16u | %2u | %9u | %9u | %9u | %14u \n", 0, L0, L1, L2,
  // L3, offset);
}

char *buffer;
void readFileNoMallocSetup() { buffer = malloc(FILE_SIZE); }
void readFileNoMalloc() { read_file_no_alloc("in.json", buffer); }

void readFile() { char *buffer = read_file("in.json"); }

#define NUMBER_OF_FAULTS 1
#define MY_PAGE_SIZE 10000 // (1024 * 16)

int *redundantMovesBuffer;
void redundantMoves1Setup() {
  redundantMovesBuffer =
      mmap(NULL, NUMBER_OF_FAULTS * MY_PAGE_SIZE, PROT_READ | PROT_WRITE,
           MAP_PRIVATE | MAP_ANON, -1, 0);
}

void redundantMoves1() {
  read_x1(redundantMovesBuffer, NUMBER_OF_FAULTS * MY_PAGE_SIZE);
}

char *switchVSConditionalPtr;
uint64_t s1 = 0xFFFFFFFF;
void switchVSConditionalSetup() {
  switchVSConditionalPtr =
      mmap(NULL, s1, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);

  char *ptr = switchVSConditionalPtr;
  char messages[10] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j'};

  for (int i = 0; i < s1; i++) {
    *ptr++ = messages[i % 10];
  }
}
void switchVSConditional() {
  char *ptr = switchVSConditionalPtr;
  int sum = 0;

  for (int i = 0; i < s1; i++) {
    switch (*ptr++) {
    case 'a': {
      sum += 1;
      /* code */
      break;
    }
    case 'b': {
      sum += 47;
      /* code */
      break;
    }
    case 'c': {
      sum += 28;
      /* code */
      break;
    }
    case 'd': {
      sum += 13;
      /* code */
      break;
    }
    case 'e': {
      sum += -1;
      /* code */
      break;
    }

    default:
      break;
    }
  }
}

char *conditionalVSSwitchPtr;
void conditionalVSSwitchSetup() {
  conditionalVSSwitchPtr =
      mmap(NULL, s1, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);

  char *ptr = conditionalVSSwitchPtr;
  char messages[10] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j'};

  for (int i = 0; i < s1; i++) {
    *ptr++ = messages[i % 10];
  }
}
void conditionalVSSwitch() {
  char *ptr = conditionalVSSwitchPtr;
  int sum = 0;

  for (int i = 0; i < s1; i++) {
    char el = *ptr++;

    if (el == 'a') {
      sum += 1;
    } else if (el == 'b') {
      sum += 47;
    } else if (el == 'c') {
      sum += 28;
    } else if (el == 'd') {
      sum += 13;
    } else if (el == 'e') {
      sum += -1;
    }
  }
}

void nPageFaults() {
  char *ptr = mmap(NULL, MY_PAGE_SIZE * NUMBER_OF_FAULTS,
                   PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);

  for (int i = 0; i < MY_PAGE_SIZE * NUMBER_OF_FAULTS; i += MY_PAGE_SIZE) {
    ptr[i] = 1;
  }
}

void onePageFault() {
  char *ptr = mmap(NULL, 1024 * 16, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANON, -1, 0);

  *ptr = 1;
}

char *touchEveryBytePtr;
uint32_t s = 0xFFFFFFFF >> 2;
void touchEveryByteSetup() {
  touchEveryBytePtr =
      mmap(NULL, s, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
}
void touchEveryByte() {
  char *ptr = touchEveryBytePtr;

  for (int i = 0; i < s; i++) {
    *ptr++ = 1;
  }
}

char *touchEveryByteWithMallocPtr;
void touchEveryByteWithMalloc() {
  touchEveryByteWithMallocPtr =
      mmap(NULL, s, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
  char *ptr = touchEveryByteWithMallocPtr;

  for (int i = 0; i < s; i++) {
    *ptr++ = 1;
  }
}

void touchEveryByteWithMallocTeardown() {
  munmap(touchEveryByteWithMallocPtr, s);
}

void fillMemory() {
  uint32_t s = 0xFFFFFFFF;

  void *ptr[100];

  for (int i = 0; i < (1 << 9); i++) {
    ptr[i] =
        mmap(NULL, s, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);

    if (ptr[i] == MAP_FAILED) {
      perror("mmap failed");
      break;
    } else {
      fprintf(stderr, "%d. %p\n", i, ptr[i]);
      // printPointerDecomposition(ptr[i]);
    }
  }
}

void NO_SETUP() {}
void NO_TEARDOWN() {}
int main(int argc, char **argv) {
  processArgs(argc, argv);

  int maxTime = 10;
  maxTime = atoi(argv[argc - 1]);

  SubectUnderTestRepetitionTester *readFileTest =
      initSubectUnderTestRepetitionTester("malloc + readFile", readFile,
                                          NO_SETUP, NO_SETUP, FILE_SIZE,
                                          maxTime);

  SubectUnderTestRepetitionTester *readFileNoMallocTest =
      initSubectUnderTestRepetitionTester("readFile", readFileNoMalloc,
                                          readFileNoMallocSetup, NO_SETUP,
                                          FILE_SIZE, maxTime);

  SubectUnderTestRepetitionTester *touchEveryByteTest =
      initSubectUnderTestRepetitionTester("touchEveryByte", touchEveryByte,
                                          touchEveryByteSetup, NO_SETUP, s,
                                          maxTime);

  SubectUnderTestRepetitionTester *touchEveryByteWithMallocTest =
      initSubectUnderTestRepetitionTester(
          "malloc + touchEveryByte", touchEveryByteWithMalloc, NO_SETUP,
          touchEveryByteWithMallocTeardown, s, maxTime);

  SubectUnderTestRepetitionTester *onePageFaultTest =
      initSubectUnderTestRepetitionTester("malloc + onePageFault", onePageFault,
                                          NO_SETUP, NO_SETUP, MY_PAGE_SIZE,
                                          maxTime);

  SubectUnderTestRepetitionTester *nPageFaultsTest =
      initSubectUnderTestRepetitionTester(
          "malloc + nPageFaults", nPageFaults, NO_SETUP, NO_SETUP,
          MY_PAGE_SIZE * NUMBER_OF_FAULTS, maxTime);

  SubectUnderTestRepetitionTester *redundantMoves1Test =
      initSubectUnderTestRepetitionTester(
          "moves test", redundantMoves1, redundantMoves1Setup, NO_TEARDOWN,
          MY_PAGE_SIZE * NUMBER_OF_FAULTS, maxTime);

  SubectUnderTestRepetitionTester *switchVSConditionalTest =
      initSubectUnderTestRepetitionTester(
          "switch vs conditional", switchVSConditional,
          switchVSConditionalSetup, NO_TEARDOWN,
          MY_PAGE_SIZE * NUMBER_OF_FAULTS, maxTime);

  SubectUnderTestRepetitionTester *conditionalVSSwitchTest =
      initSubectUnderTestRepetitionTester(
          "conditional vs switch", conditionalVSSwitch,
          conditionalVSSwitchSetup, NO_TEARDOWN,
          MY_PAGE_SIZE * NUMBER_OF_FAULTS, maxTime);

  subjects[0] = switchVSConditionalTest;
  subjects[1] = conditionalVSSwitchTest;
  subjects[2] = EMPTY_SUBJECT_UNDER_TEST_REPETITION_TESTER;
  subjects[3] = EMPTY_SUBJECT_UNDER_TEST_REPETITION_TESTER;

  runRepetitionTester();

  printResultsRepetitiontester();

  return 0;
}
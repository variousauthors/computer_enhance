#include <alloca.h>
#include <stdio.h>
#include <string.h>

#include <CoreServices/CoreServices.h>
#include <mach/mach.h>
#include <mach/mach_time.h>

#include "global.c"
#include "global.h"
#include "haversine_formula.c"
#include "haversine_formula.h"
#include "json.c"
#include "json.h"
#include "json_parser.c"
#include "json_parser.h"
#include "json_tokenizer.c"
#include "json_tokenizer.h"
#include "os_metrics.c"
#include "os_metrics.h"

#define EARTH_RADIUS 6317

char peek() {
  char c = getc(source);
  ungetc(c, source);

  return c;
}

Token toTokenChar(char c) {
  Token result;

  return result;
}

typedef struct HaversinePair {
  double x0, y0;
  double x1, y1;
} HaversinePair;

int makeHaversinePairs(JSONNode *pairs, HaversinePair *result) {
  TimeFunction;
  JSONNode *element = pairs->value;
  uint64_t i = 0;

  do {
    float x0 = getValueByKey(element->value, "x0")->scalar.number;
    float x1 = getValueByKey(element->value, "x1")->scalar.number;
    float y0 = getValueByKey(element->value, "y0")->scalar.number;
    float y1 = getValueByKey(element->value, "y1")->scalar.number;

    result[i++] = (HaversinePair){x0, x1, y0, y1};
  } while ((element = element->next));

  return i;
}

void sumHaversine(uint64_t count, HaversinePair *pairs) {
  TimeBandwidth(count * sizeof(HaversinePair));

  double avg = 0;

  for (int i = 0; i < count; i++) {
    HaversinePair pair = pairs[i];
    double h =
        ReferenceHaversine(pair.x0, pair.y0, pair.x1, pair.y1, EARTH_RADIUS);
    avg += h;
  }

  avg /= count;

  fprintf(stderr, "Input size: %d\n", 123);
  fprintf(stderr, "Pair count: %lld\n", count);
  fprintf(stderr, "Haversine sum: %f\n", avg);
}

int main(int argc, char **argv) {

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-v") == 0) {
      verbose = 1;
    }

    if (strcmp(argv[i], "-perf") == 0) {
      perf = 1;
    }

    if (strcmp(argv[i], "-emit") == 0) {
      emit = 1;
    }
  }

  if (verbose) {
    verboseChannel = stderr;
    perfChannel = fopen("/dev/null", "w");
  } else if (perf) {
    perfChannel = stderr;
    verboseChannel = fopen("/dev/null", "w");
  } else {
    verboseChannel = fopen("/dev/null", "w");
    perfChannel = fopen("/dev/null", "w");
  }

  beginProfiler();

  if (perf) {
    perfChannel = stderr;
  } else {
    perfChannel = fopen("/dev/null", "w");
  }

  char *answers = argv[argc - 1];
  char *in = argv[argc - 2];
  source = fopen(in, "rb");
  Token next;

  JSONNode *root = parseJSON();

  // this is the "value node"
  JSONNode *pairs = getValueByKey(root, "pairs");

  HaversinePair *haversinePairs = malloc(pairs->length * sizeof(HaversinePair));
  uint64_t count = makeHaversinePairs(pairs, haversinePairs);

  sumHaversine(count, haversinePairs);

  endAndPrintProfiler();
}

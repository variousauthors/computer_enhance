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

void sumHaversine(JSONNode *pairs) {
  TimeFunction;

  JSONNode *element = pairs->value;

  if (element == 0) {
    fprintf(verboseChannel, "empty\n");
  } else {
    fprintf(verboseChannel, "values\n");
    double avg = 0;
    long count = 0;

    do {
      // each element is a k/v pair like { "0": { "x0": 1, "x1": 2, ... } }
      // so we getValueByKey of emelement->value
      fprintf(verboseChannel, "type: %s, key: %s \n",
              toStringJSONType(element->type), element->key);

      float x0 = getValueByKey(element->value, "x0")->scalar.number;
      float x1 = getValueByKey(element->value, "x1")->scalar.number;
      float y0 = getValueByKey(element->value, "y0")->scalar.number;
      float y1 = getValueByKey(element->value, "y1")->scalar.number;

      fprintf(verboseChannel,
              "    { \"x0\": %f, \"y0\": %f, \"x1\": %f, \"y1\": %f }%s\n", x0,
              y0, x1, y1, element->next ? "," : "");

      double h = ReferenceHaversine(x0, y0, x1, y1, EARTH_RADIUS);
      avg += h;
      count++;

    } while ((element = element->next));

    avg /= count;

    fprintf(stderr, "Input size: %d\n", 123);
    fprintf(stderr, "Pair count: %ld\n", count);
    fprintf(stderr, "Haversine sum: %f\n", avg);
  }
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

  fprintf(verboseChannel, "hello\n");
  // this is the "value node"
  JSONNode *pairs = getValueByKey(root, "pairs");

  fprintf(verboseChannel, "%s\n", toStringJSONType(pairs->type));

  sumHaversine(pairs);

  endAndPrintProfiler();
}

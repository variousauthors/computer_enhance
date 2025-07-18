#include "haversine_formula.c"
#include "haversine_formula.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EARTH_RADIUS 6317

int cluster = 0;

int main(int argc, char *argv[]) {
  char *endptr;
  char *countString = argv[argc - 1];

  long count = strtol(countString, &endptr, 10);

  if (*endptr != '\0') {
    fprintf(stderr, "Invalid characters after number: %s\n", endptr);
  }

  char *seedString = argv[argc - 2];
  endptr = 0;

  long seed = strtol(seedString, &endptr, 10);

  if (*endptr != '\0') {
    fprintf(stderr, "Invalid characters after number: %s\n", endptr);
  }

  // do the args
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "uniform") == 0) {
      cluster = 0;
    }

    if (strcmp(argv[i], "cluster") == 0) {
      cluster = 1;
    }
  }

  FILE *json = fopen("out.json", "w");
  FILE *answers = fopen("out.f64", "wb");

  fprintf(stderr, "seed: %ld, count: %ld, cluster: %d\n", seed, count, cluster);

  srand(seed);

  fprintf(json, "{\n");
  fprintf(json, "  \"pairs\": [\n");

  double avg = 0;

  // store the count in the f64 file
  fwrite(&count, sizeof(count), 1, answers);

  for (int i = 0; i < count; i++) {
    float x0 = (float)rand() / (float)(RAND_MAX / 360) - 180;
    float y0 = (float)rand() / (float)(RAND_MAX / 360) - 180;
    float x1 = (float)rand() / (float)(RAND_MAX / 180) - 90;
    float y1 = (float)rand() / (float)(RAND_MAX / 180) - 90;

    double h = ReferenceHaversine(x0, y0, x1, y1, EARTH_RADIUS);
    avg += h;

    fprintf(stderr, "%f, ", h);
    fwrite(&h, sizeof(h), 1, answers);

    fprintf(json, "    { \"x0\": %f, \"y0\": %f, \"x1\": %f, \"y1\": %f }%s\n",
            x0, y0, x1, y1, i < count - 1 ? "," : "");
  }

  avg /= count;
  fwrite(&avg, sizeof(avg), 1, answers);

  fprintf(json, "  ]\n");
  fprintf(json, "}\n");

  fprintf(stderr, "Method: %s\n", cluster ? "cluster" : "uniform");
  fprintf(stderr, "Random seed: %ld\n", seed);
  fprintf(stderr, "Pair count: %ld\n", count);
  fprintf(stderr, "Expected sum: %f\n", avg);
}
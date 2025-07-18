#include "haversine_formula.c"
#include "haversine_formula.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
  double result = ReferenceHaversine(0, 0, 90, 90, 6371);
  fprintf(stderr, "result: %f\n", result);
}
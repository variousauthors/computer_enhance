#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdio.h>

FILE *verboseChannel;
FILE *perfChannel;

int emit;
int verbose;
int perf;

#define TimeFunction 1

void beginProfiler();

#endif
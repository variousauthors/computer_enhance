zig cc -target aarch64-macos -c src/asm/multi-read.s -o src/lib/multi-read.o

zig cc -g -O1 src/haversine_generator.c -o havrsine_generator
zig cc -g -O1 src/haversine.c -o haversine
zig cc -g -O1 src/main.c src/lib/multi-read.o -o main

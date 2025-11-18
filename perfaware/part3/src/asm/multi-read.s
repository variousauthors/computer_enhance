    .globl _read_x1
    .text
    .align 2

_read_x1:
  .p2align 6
.loop:
  ldr w8, [x0], #4
  subs x1, x1, #1
  bgt .loop
  mov w0, w8 
  ret
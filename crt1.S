  .text
  .global _start
_start:
  xor %rbp, %rbp # clear the frame pointer
  pop %rdi       # argc
  mov %rsp, %rsi # argv
  call load      # call ld.so entry function
  hlt            # `load` should `exit`. If that doesn't happen, halt.
.L_start_end:
  .size _start, .L_start_end-_start

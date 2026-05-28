section .text
global _start
extern main

_start:
    call main

_halt:
    hlt
    jmp _halt

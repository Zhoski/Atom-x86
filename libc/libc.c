#include "libc.h"

void print(unsigned const char* str) {
    asm("movl $1, %%eax\n"
        "movl $1, %%ebx\n"
        "movl %0, %%ecx\n"
        "int $0x80"
            :
            : "r" (str)
            : "%eax", "%ebx", "%ecx"
        );
}

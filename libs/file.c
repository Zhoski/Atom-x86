#include "file.h"

unsigned char cat(const unsigned char* file_name, unsigned char buffer[512]) {
    asm volatile(
        "movl $5, %%eax\n"
        "movl $1, %%ebx\n"
        "movl %0, %%ecx\n"
        "movl %1, %%edx\n"
        "int $0x80"
        :
        : "r"(file_name), "r" (buffer)
        : "eax", "ebx", "ecx", "edx", "memory"
    );
}
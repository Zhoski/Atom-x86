#include "file.h"

unsigned char cat(const unsigned char* file_name, unsigned char *out) {
    asm volatile(
        "movl $5, %%eax\n"
        "movl $1, %%ebx\n"
        "movl %[file], %%ecx\n"
        "movl %[o], %%edx\n"
        "int $0x80"
        :
        : [file] "r"(file_name), [o] "r" (out)
        : "eax", "ebx", "ecx", "edx", "memory"
    );
}

unsigned char get_root(unsigned char *__restrict__ out) {
    asm volatile(
        "movl $5, %%eax\n"
        "movl $2, %%ebx\n"
        "movl %0, %%ecx\n"
        "int $0x80"
        :
        : "r"((unsigned int)out)
        : "eax", "ebx", "ecx", "memory"
    );
}

unsigned int sys_run(unsigned char* __restrict__ file) {
    asm volatile(
        "movl $6, %%eax\n"
        "movl $1, %%ebx\n"
        "movl %0, %%ecx\n"
        "int $0x80"
        :
        : "r"((unsigned int)file)
        : "eax", "ebx", "ecx", "memory"
    );
}

unsigned int sys_died() {
    asm volatile(
        "movl $60, %%eax\n"
        "int $0x80"
        :
        : 
    );
}
#include "file.h"

unsigned int sys_read(const unsigned char* __restrict__ file_name, unsigned char* __restrict__ out) {
    int ret = 0;
    asm volatile(
        "movl $5, %%eax\n"
        "movl $1, %%ebx\n"
        "movl %[file], %%esi\n"
        "movl %[o], %%edx\n"
        "int $0x80"
        :
        : [file] "r"(file_name), [o] "r" (out)
        : "eax", "ebx", "esi", "edx", "memory"
    );
    asm volatile(
        "movl %0, %%eax"
        : "=a" (ret)
    );
    return ret;
}

unsigned int sys_check(const unsigned char* __restrict__ file_name) {
    int ret = 0;
    asm volatile(
        "movl $5, %%eax\n"
        "movl $6, %%ebx\n"
        "movl %[file], %%esi\n"
        "int $0x80"
        :
        : [file] "r"(file_name)
        : "eax", "ebx", "ecx", "edx", "memory"
    );
    asm volatile(
        "movl %0, %%eax"
        : "=a" (ret)
    );
    return ret;
}

unsigned int sys_write(const unsigned char* __restrict__ file_name, unsigned char* __restrict__ in, unsigned int count) {
    int ret = 0;
    asm volatile(
        "movl $5, %%eax\n"
        "movl $3, %%ebx\n"
        "movl %[c], %%ecx\n"
        "movl %[file], %%esi\n"
        "movl %[in], %%edi\n"
        "int $0x80"
        :
        : [c] "m" (count), [file] "m" ((int)file_name), [in] "m" ((int)in)
        : "eax", "ebx", "ecx", "esi", "edi", "memory"
    );
    asm volatile(
        "movl %0, %%eax"
        : "=a" (ret)
    );
}

unsigned char sys_create(const unsigned char* __restrict__ file_name) {
    asm volatile(
        "movl $5, %%eax\n"
        "movl $4, %%ebx\n"
        "movl %[file], %%esi\n"
        "int $0x80"
        :
        : [file] "m" ((int)file_name)
        : "eax", "ebx", "ecx", "esi", "edi", "memory"
    );
}

unsigned char sys_delete(const unsigned char* __restrict__ file_name) {
    int ret = 0;
    asm volatile(
        "movl $5, %%eax\n"
        "movl $5, %%ebx\n"
        "movl %[file], %%esi\n"
        "int $0x80"
        :
        : [file] "m" ((int)file_name)
        : "eax", "ebx", "ecx", "esi", "edi", "memory"
    );
    asm volatile(
        "movl %0, %%eax"
        : "=a" (ret)
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

unsigned int sys_run(const unsigned char* __restrict__ file) {
    int ret = 0;
    asm volatile(
        "movl $5, %%eax\n"
        "movl $7, %%ebx\n"
        "movl %0, %%esi\n"
        "int $0x80"
        :
        : "r"((unsigned int)file)
        : "eax", "ebx", "esi", "memory"
    );
    asm volatile(
        "movl %0, %%eax"
        : "=a" (ret)
    );

    return ret;
}

unsigned int sys_died() {
    asm volatile(
        "movl $60, %%eax\n"
        "int $0x80"
        :
        : 
    );
}

File* fopen(unsigned char *__restrict__ file, const unsigned int mode) {
    File* ret;
    asm volatile(
        "movl $5, %%eax\n"
        "movl $8, %%ebx\n"
        "movl %0, %%esi\n"
        "int $0x80"
        :
        : "r"((unsigned int)file)
        : "eax", "ebx", "esi", "memory"
    );

    asm volatile(
        "movl $5, %%eax\n"
        "movl $1, %%ebx\n"
        "movl %[file], %%esi\n"
        "movl %[o], %%edx\n"
        "int $0x80"
        :
        : [file] "r"(file), [o] "r" (ret->base)
        : "eax", "ebx", "esi", "edx", "memory"
    );

    asm volatile(
        "movl %0, %%eax"
        : "=a" (ret)
    );

    return ret;
}

void fclose(File* file) {
    asm volatile(
        "movl $5, %%eax\n"
        "movl $1, %%ebx\n"
        "movl %[file], %%esi\n"
        "int $0x80"
        :
        : [file] "r"(file)
        : "eax", "ebx", "esi", "memory"
    );
}

void fread(File* stream, unsigned int n, unsigned char* out) {
    
}
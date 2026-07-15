#include "memory.h"

unsigned char memread(unsigned char *__restrict__ from) {
    return from[0];
}

unsigned short memread_dw(unsigned short *__restrict__ from) {
    return from[0];
}

unsigned int memread_dd(unsigned int *__restrict__ from) {
    return from[0];
}

void memset(unsigned char *__restrict__ in, unsigned char v, unsigned int c) {
    asm volatile (
        "movl %[v], %%eax\n"
        "movl %[in], %%edi\n"
        "movl %[count], %%ecx\n"
        "rep stosb"
        :
        : [in] "r" ((unsigned char*)in), [v] "r" ((unsigned int)v), [count] "r" (c)
        : "eax", "edi", "ecx", "memory"
    );
}

void memcpy(void *__restrict__ from,void *__restrict__ in, unsigned int size) {
    asm volatile (
        "movl %[from], %%esi\n"
        "movl %[in],   %%edi\n"
        "movl %[count],%%ecx\n"
        "rep movsb"
        :
        : [from] "r" ((unsigned char*)from), [in] "r" ((unsigned char*)in), [count] "r" (size)
        : "esi", "edi", "ecx","memory"
    ); 
}

unsigned char* malloc(unsigned int size) {
    asm volatile(
        "movl $4, %%eax\n"
        "movl $6, %%ebx\n"
        "movl %0, %%ecx\n"
        "int $0x80"
        :
        : "r"(size)
        : "eax", "ebx", "ecx", "memory"
    );

    unsigned int free_adres;

    asm volatile (
        "movl %%eax, %0"
        : "=r" (free_adres)
        :
        : "memory"
    );

    return (unsigned char*)free_adres;
}

unsigned char free(unsigned char *__restrict__ data) {
    asm volatile(
        "movl $4, %%eax\n"
        "movl $7, %%ebx\n"
        "movl %0, %%ecx\n"
        "int $0x80"
        :
        : "r"((unsigned int)data)
        : "eax", "ebx", "ecx", "memory"
    );
}
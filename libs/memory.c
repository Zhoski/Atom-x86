#include "memory.h"
#include "strio.h"

unsigned char memread(unsigned char* from) {
    return from[0];
}

unsigned short memread_dw(unsigned short* from) {
    return from[0];
}

unsigned int memread_dd(unsigned int* from) {
    return from[0];
}

void memset(unsigned char* in, unsigned char v, unsigned int size) {
    for(unsigned int i = 0;i < size;i++) {
        *in++ = v;
    }
}

void memcpy(void* from,void* in, unsigned int size) {
    unsigned char *from_byte = (unsigned char*)from;
    unsigned char *in_byte = (unsigned char*)in;
    for(int i = 0;i < size;i++)  {
        *in_byte++ = *from_byte++; 
    }   
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

unsigned char free(unsigned char* data) {
    asm volatile(
        "movl $4, %%eax\n"
        "movl $7, %%ebx\n"
        "movl %0, %%ecx\n"
        "int $0x80"
        :
        : "r"((uint32)data)
        : "eax", "ebx", "ecx", "memory"
    );
}
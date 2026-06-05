#include "memory.h"

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
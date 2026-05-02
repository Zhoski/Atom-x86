#include "memory.h"
#include <stdint.h>

void memcpy(int* from,int* in, int size) {
    for(int i = 0;i < size;i++)  {
        *in++ = *from++; 
    }   
}

uint8_t memread(uint8_t* from) { 
    return from[0];
}

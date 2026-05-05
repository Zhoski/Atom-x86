#include "memory.h"
#include "../services.h"
#include <stdint.h>

static memory mem = {
    .memcpy = &memcpy,
    .memread = &memread,
    .memread_dd = &memread_dd,
};

void init_memory() {
    service.memory = &mem;
}

void memcpy(uint8_t* from,uint8_t* in, uint32_t size) {
    for(int i = 0;i < size;i++)  {
        *in++ = *from++; 
    }   
}

uint8_t memread(uint8_t* from) {
    return from[0];
}

uint32_t memread_dd(uint32_t* from) {
    return from[0];
}

#ifndef __MEMORY__
#define __MEMORY__
#include <stdint.h>

void memcpy(void* from,void* in, uint32_t size);
uint8_t memread(uint8_t* from);
uint16_t memread_dw(uint16_t* from);
void memset(uint8_t* in, uint8_t v, uint32_t size);
uint32_t memread_dd(uint32_t* from);
uint8_t* malloc(uint32_t size);
void free(uint8_t *ptr);
void create_heap();
void init_memory();
#endif

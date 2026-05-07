#ifndef __MEMORY__
#define __MEMORY__
#include <stdint.h>
extern void memcpy(void* from,void* in, uint32_t size);
extern uint8_t memread(uint8_t* from);
extern uint32_t memread_dd(uint32_t* from);
extern void init_memory();
#endif

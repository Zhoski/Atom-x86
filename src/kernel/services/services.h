#ifndef __SERVICES__
#define __SERVICES__
#include <stdint.h>

typedef struct {
   void (*memcpy)(void* from,void* in, uint32_t size);
   uint8_t (*memread)(uint8_t*);
   uint16_t (*memread_dw)(uint16_t*);
   uint32_t (*memread_dd)(uint32_t*);
   void (*memset)(uint8_t*, uint8_t v, uint32_t);
   uint8_t* (*malloc)(uint32_t);
   void (*free)(uint8_t*);
   void (*create_heap)();
} memory;

typedef struct {
    memory* memory;
} services;

extern services service;

#endif

#ifndef __ALLOCATE__
#define __ALLOCATE__
#include <stdint.h>

extern uint32_t malloc_stack();
extern uint32_t malloc_page();

extern void init_allocate();

#endif

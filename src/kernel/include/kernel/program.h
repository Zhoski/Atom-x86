#ifndef __PROGRAM__
#define __PROGRAM__
#include <stdint.h>

extern uint32_t kernel_stack_ptr;
extern uint32_t kernel_stack_base;
extern uint32_t kernel_return_ptr;

void program_execute(uint32_t entry, uint32_t stack); 

#endif

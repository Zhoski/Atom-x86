#ifndef __MEMORY_MAP__
#define __MEMORY_MAP__
#include <stdint.h>

static inline uint8_t op_on(uint8_t byte, uint8_t index) { return byte | (1 << index); }
static inline uint8_t op_off(uint8_t byte, uint8_t index) { return byte & ~(1 << index); }
extern uint8_t (*bit_op[2])(uint8_t, uint8_t);
extern uint8_t (*stack_bit_op[2])(uint8_t, uint8_t);
extern void free_page(uint32_t page_adres);
extern uint32_t malloc_stack();
#endif

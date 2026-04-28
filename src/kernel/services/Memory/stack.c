#include "memory_map.h"
#include <stdint.h>

// Стек
const uint32_t stack_base_adres = 0x100000;   // Начиная отсюда 
const uint32_t stack_block_size = 0x1000;     // 4 Kib
const uint32_t stack_block_count = 0x1000;    // 4096 блоков

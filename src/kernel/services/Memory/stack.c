#include "memory_map.h"
#include <stdint.h>

#define USED    1
#define FREE    0

// Стек
const uint32_t stack_base_adres = 0x100000;     // Начиная отсюда 
const uint32_t stack_block_size = 0x2000;       // 8 Kib
const uint32_t stack_block_count = 128;         // 128 блоков
uint8_t stack_mem_bit_mask[16];                 // Массив из 128 бит 

uint8_t (*stack_bit_op[2])(uint8_t, uint8_t) = { op_off, op_on };

void stack_set_mem_bit_mask(uint32_t byte, uint8_t bit, uint8_t(operation)(uint8_t, uint8_t)) {
    // byte номер байта в маске битов
    // bit номер бита в байте
    // state либо 0 либо 1

    uint8_t* stack_mem_bit_mask_ptr = &stack_mem_bit_mask[byte];
    uint8_t _byte = *stack_mem_bit_mask_ptr;
    _byte = operation(_byte, bit);
    *stack_mem_bit_mask_ptr = _byte;
}

uint32_t find_free_stack_block() {
    uint32_t byte = 0;
    uint32_t _bit = 0;
    uint32_t out = 0;
    while (stack_mem_bit_mask[byte++] == 0xFF);  // Теперь i равен номеру байта в котом находится свободный бит
    // Теперь надо найти какой именно бит равен 0
    uint8_t temp = 1;
    uint8_t bit = stack_mem_bit_mask[--byte];
    for (_bit = 0;_bit < 8;_bit++) {
        if (!(bit & temp)) {
            out = byte * 8 + _bit;           // Вот свободная ячейка
            break;
        }
        temp <<= 1;                     // Сдвиг влево
    }
    stack_set_mem_bit_mask(byte, _bit, stack_bit_op[USED]); // Установить бит в 1
    return out;
}

uint32_t mallock_stack() {
    uint32_t adres = find_free_stack_block() * stack_block_size + stack_base_adres;
    return adres;
}

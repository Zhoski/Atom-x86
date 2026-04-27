#include "memory_managment.h"
#include <stdint.h>

#define USED    1
#define FREE    0

const uint32_t base_adres = 0x100000;   // Начиная отсюда 
const uint32_t block_size = 0x1000;     // 4 Kib
const uint32_t block_count = 0x1000;    // 4096 блоков

uint8_t mem_bit_mask[4096];  // Массив из 4096 бит 

uint8_t op_on(uint8_t byte, uint8_t index) {
    return byte | (1 << index);
}

uint8_t op_off(uint8_t byte, uint8_t index) {
    return byte & ~(1 << index);
}

uint8_t(*bit_op[2])(uint8_t, uint8_t) = {op_off, op_on};

void set_mem_bit_mask(uint32_t byte, uint8_t bit, uint8_t(operation)(uint8_t, uint8_t)) {
    // byte номер байта в маске битов
    // bit номер бита в байте
    // state либо 0 либо 1

    uint8_t* mem_bit_mask_ptr = &mem_bit_mask[byte];
    uint8_t _byte = *mem_bit_mask_ptr;
    _byte = operation(_byte, bit);
    *mem_bit_mask_ptr = _byte;
}

uint32_t find_free_page() {
    uint32_t byte = 0;
    uint32_t _bit = 0;
    uint32_t out = 0;
    while (mem_bit_mask[byte++] == 0xFF);   // Теперь i равен номеру байта в котом находится свободный бит
    // Теперь надо найти какой именно бит равен 0
    uint8_t temp = 1;
    uint8_t bit = mem_bit_mask[--byte];
    for (_bit = 0;_bit < 8;_bit++) {
        if (!(bit & temp)) {
            out = byte * 8 + _bit;           // Вот свободная ячейка
            break;
        }
        temp <<= 1;                     // Сдвиг влево
    }
    set_mem_bit_mask(byte, _bit, bit_op[USED]); // Установить бит в 1
    return out;
}

uint32_t malloc_page() {
    uint32_t adress = find_free_page() * block_size + base_adres;
    return adress;
}



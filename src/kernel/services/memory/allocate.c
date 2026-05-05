#include "allocate.h"
#include "../services.h"

#define STACK_BASE  0x200000
#define PAGE_BASE   0x300000

#define STACK_SIZE  0x2000      // 8Kib
#define PAGE_SIZE   0x1000      // 4Kib

#define PAGE_COUNT  4096
#define STACK_COUNT 128

#define USED    1
#define FREE    0

static inline uint8_t op_on(uint8_t byte, uint8_t index) { return byte | (1 << index); }
static inline uint8_t op_off(uint8_t byte, uint8_t index) { return byte & ~(1 << index); }

allocate _allocate = {
    .malloc_page = &malloc_page,
    .malloc_stack = &malloc_stack,
};

void init_allocate() {
    service.allocate = &_allocate;
};

// ====================== Стек =====================
const uint32_t stack_base_adres = STACK_BASE;     // Начиная отсюда 
const uint32_t stack_block_size = STACK_SIZE;       // 8 Kib
const uint32_t stack_block_count = STACK_COUNT;         // 128 блоков
uint8_t stack_mem_bit_mask[16];                 // Массив из 128 бит 

// ======================== Страницы ========================
const uint32_t page_base_adres = PAGE_BASE;   // Начиная отсюда 
const uint32_t page_block_size = PAGE_SIZE;   // 4 Kib
const uint32_t page_block_count = PAGE_COUNT; // 4096 блоков
uint8_t page_mem_bit_mask[512]; 

uint8_t (*bit_op[2])(uint8_t, uint8_t) = { op_off, op_on };

void set_mem_bit_mask(uint32_t byte, uint8_t bit, uint8_t(operation)(uint8_t, uint8_t)) {
    // byte номер байта в маске битов
    // bit номер бита в байте
    // state либо 0 либо 1

    uint8_t* mem_bit_mask_ptr = &page_mem_bit_mask[byte];
    uint8_t _byte = *mem_bit_mask_ptr;
    _byte = operation(_byte, bit);
    *mem_bit_mask_ptr = _byte;
}

void set_mem_bit_mask_stack(uint32_t byte, uint8_t bit, uint8_t(operation)(uint8_t, uint8_t)) {
    // byte номер байта в маске битов
    // bit номер бита в байте
    // state либо 0 либо 1

    uint8_t* mem_bit_mask_ptr = &stack_mem_bit_mask[byte];
    uint8_t _byte = *mem_bit_mask_ptr;
    _byte = operation(_byte, bit);
    *mem_bit_mask_ptr = _byte;
}

uint32_t find_free_page() {
    uint32_t byte = 0;
    uint32_t _bit = 0;
    uint32_t out = 0;
    while (page_mem_bit_mask[byte++] == 0xFF);   // Теперь i равен номеру байта в котом находится свободный бит
    // Теперь надо найти какой именно бит равен 0
    uint8_t temp = 1;
    uint8_t bit = page_mem_bit_mask[--byte];
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
    uint32_t adress = find_free_page() * page_block_size + page_base_adres;
    return adress;
}

// ======================= Стек ==========================

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
    set_mem_bit_mask_stack(byte, _bit, bit_op[USED]); // Установить бит в 1
    return out;
}

uint32_t malloc_stack() {
    uint32_t adres = find_free_stack_block() * stack_block_size + stack_base_adres;
    return adres;
}

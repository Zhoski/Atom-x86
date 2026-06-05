#include "memory.h"
#include "../services.h"
#include <stdint.h>

/* ---------- Куча ---------- */
#define HEAP_BASE           0x200000                // 2Мб
#define HEAP_SIZE           0x100000                // 1Мб

#define FREE                 1
#define BUSY                 0
#define NULL                 0
#define MEMORY_NOT_FOUND    -1

static memory mem = {
    .memcpy = &memcpy,
    .memread = &memread,
    .memread_dw = &memread_dw,
    .memread_dd = &memread_dd,
    .memset = &memset,
};

typedef struct {
    int size;
    char is_free;
    struct Heap* next;
} Heap;

Heap* block_heap = (Heap*)HEAP_BASE;

uint8_t* malloc(uint32_t size) {
    // offset_in_vmem теперь становится реальным физическим адресом в ОЗУ
    uint32_t current_address = HEAP_BASE + sizeof(Heap) + size;
    Heap* current = block_heap;
    
    while (current != NULL)
    {
        uint32_t heap_end_address = HEAP_BASE + HEAP_SIZE;

        if (current_address + sizeof(Heap) >= heap_end_address) {
            return MEMORY_NOT_FOUND;
        }

        if (current->size >= (size + sizeof(Heap)) && current->is_free == FREE) {
            Heap new_heap = {
                .is_free = FREE,
                .size = current->size - size - sizeof(Heap),
            };

            current->size = size;
            current->is_free = BUSY;

            if (current->next != NULL) {
                new_heap.next = current->next;
            }
            else {
                new_heap.next = NULL;
            }           

            uint8_t* t = (uint8_t*)&new_heap;
            uint8_t* dest = (uint8_t*)current_address;
            for (int i = 0; i < 12; i++) {
                dest[i] = t[i];
            }

            current->next = (struct Heap*)current_address;
            
            return (uint8_t*)(current_address - size);
        }

        current_address += (current->size + sizeof(Heap));
        current = current->next;
    }

    return MEMORY_NOT_FOUND;
}

void free(uint8_t *ptr) {
    uint8_t* heap = ptr - sizeof(Heap);
    uint8_t* next = heap;

    heap[4] = FREE;
    
    next += *(int*)heap + sizeof(Heap);

    while (next[4] == FREE) {
        int new_size = *(int*)next + *(int*)heap + sizeof(Heap);
        uint8_t* t = (int*)&new_size;
        for (uint32_t i = 0;i < 4;i++) {
            heap[i] = t[i];
        }
         
        for (uint32_t i = 0;i < 4;i++) {
            heap[8 + i] = next[8 + i];
        }

        next += *(int*)next + sizeof(Heap);
    }

    ptr = NULL;      // Уничтожение указателя
}

void init_memory() {
    service.memory = &mem;
}

void memset(uint8_t* in, uint8_t v, uint32_t size) {
    for(uint32_t i = 0;i < size;i++) {
        *in++ = v;
    }
}

void memcpy(void* from,void* in, uint32_t size) {
    uint8_t *from_byte = (uint8_t*)from;
    uint8_t *in_byte = (uint8_t*)in;
    for(int i = 0;i < size;i++)  {
        *in_byte++ = *from_byte++; 
    }   
}

uint8_t memread(uint8_t* from) {
    return from[0];
}

uint16_t memread_dw(uint16_t* from) {
    return from[0];
}

uint32_t memread_dd(uint32_t* from) {
    return from[0];
}

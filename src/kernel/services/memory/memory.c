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
    .malloc = &malloc,
    .free = &free,
    .create_heap = &create_heap
};

typedef struct {
    int size;
    char is_free;
    struct Heap* next;
} Heap;

Heap* block_heap = (Heap*)HEAP_BASE;

void create_heap() {
    block_heap->is_free = FREE;
    block_heap->size = HEAP_SIZE;
    block_heap->next = NULL;
}

U8* malloc(U32 size) {
    U32 current_address = HEAP_BASE + sizeof(Heap) + size;
    Heap* current = block_heap;
    
    while (current != NULL)
    {
        U32 heap_end_address = HEAP_BASE + HEAP_SIZE;

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

            U8* t = (U8*)&new_heap;
            U8* dest = (U8*)current_address;
            for (int i = 0; i < 12; i++) {
                dest[i] = t[i];
            }

            current->next = (struct Heap*)current_address;
            
            return (U8*)(current_address - size);
        }

        current_address += (current->size + sizeof(Heap));
        current = current->next;
    }

    return MEMORY_NOT_FOUND;
}

void free(U8* __restrict__ ptr) {
    U8* heap = ptr - sizeof(Heap);
    U8* next = heap;

    heap[4] = FREE;
    
    next += *(U32*)heap + sizeof(Heap);

    while (next[4] == FREE) {
        U32 new_size = *(U32*)next + *(U32*)heap + sizeof(Heap);
        U8* t = (U32*)&new_size;
        for (U32 i = 0;i < 4;i++) {
            heap[i] = t[i];
        }
         
        for (U32 i = 0;i < 4;i++) {
            heap[8 + i] = next[8 + i];
        }

        next += *(U32*)next + sizeof(Heap);
    }

    ptr = NULL;      // Уничтожение указателя
}

void init_memory() {
    service.memory = &mem;
}

void memset(void* __restrict__ in, U8 v, U32 c) {
    asm volatile (
        "movl %[v], %%eax\n"
        "movl %[in], %%edi\n"
        "movl %[count], %%ecx\n"
        "rep stosb"
        :
        : [in] "r" ((U8*)in), [v] "r" ((U32)v), [count] "r" (c)
        : "eax", "edi", "ecx", "memory"
    );
}

void memcpy(void* __restrict__ from,void* __restrict__ in, uint32_t size) {
    asm volatile (
        "movl %[from], %%esi\n"
        "movl %[in],   %%edi\n"
        "movl %[count],%%ecx\n"
        "rep movsb"
        :
        : [from] "r" ((U8*)from), [in] "r" ((U8*)in), [count] "r" (size)
        : "esi", "edi", "ecx","memory"
    );
}

U8 memread(U8* __restrict__ from) {
    return *from;
}

U16 memread_dw(U16* __restrict__ from) {
    return *from;
}

U32 memread_dd(U32* __restrict__ from) {
    return *from;
}

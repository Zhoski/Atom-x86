#include "memory_map.h"
#include "memory.h"
#include "process.h"
#include <stdint.h>

void program_execute(uint32_t entry, uint32_t stack) {
    asm volatile (
        "movl %1, %%esp\n\t"  
        "pushl $0x08\n\t"     
        "pushl %0\n\t"        
        "lretl"               
        : 
        : "r" (entry), "r" (stack)
        : "memory"
    );
}

// Спавн программы
void program_spawn(uint32_t entry_in_ram) {
    uint32_t entry = malloc_page();             
    uint32_t stack_top = malloc_stack()+0x2000; 
    memcpy((int*)entry_in_ram, (int*)entry, 4096);
    process_spawn(entry, stack_top);
    program_execute(entry, stack_top);
}

#include "memory_map.h"
#include "memory.h"
#include "process.h"
#include <stdint.h>
uint32_t kernel_stack_ptr;
uint32_t kernel_return_ptr;
void program_execute(uint32_t entry, uint32_t stack) { 
    kernel_return_ptr = (uint32_t)&&exit_ptr;
   
    asm volatile("movl %%esp, %0":"=r" (kernel_stack_ptr));

    asm volatile (   
        "movl %1, %%esp\n"   
        "jmp *%0"               
        : 
        : "r" (entry), "r" (stack)
        : "esp"
    );


exit_ptr:
    asm volatile("movl %%esp, %0":"=r" (kernel_stack_ptr));

    for(;;) {
        asm("hlt");
    } 

}

// Спавн программы
void program_spawn(uint32_t entry_in_ram) {
    uint32_t entry = malloc_page();             // Точка входа      
    uint32_t stack_top = malloc_stack()+0x2000; // Вершина стека

    // Копируем программу из entry_in_ram в entry
    memcpy((uint8_t*)entry_in_ram, (uint8_t*)entry, 4096);

    // Создаем процесс
    process_spawn(entry, stack_top);

    // На исполнение
    program_execute(entry, stack_top); 
}

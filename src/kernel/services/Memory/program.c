#include "memory_map.h"
#include "memory.h"
#include "process.h"
#include <stdint.h>
uint32_t kernel_stack_ptr;
uint32_t kernel_return_ptr;
void program_execute(uint32_t entry, uint32_t stack) { 
    kernel_return_ptr = (uint32_t)&&exit_ptr;
   
    asm volatile("movl %%esp, %0":"=r" (kernel_stack_ptr));

    kwrite_string("Kernel esp: ");
    kwrite_int(kernel_stack_ptr);
    kwrite_string("\nKernel return: ");
    kwrite_int(kernel_return_ptr);
    kwrite_string("\n");

    asm volatile (   
        "movl %1, %%esp\n"   
        "jmp *%0"               
        : 
        : "r" (entry), "r" (stack)
        : "esp"
    );


exit_ptr:
    kwrite_string("Process died\n");
    kwrite_string("Current kernel stack (in kernel): ");
    asm volatile("movl %%esp, %0":"=r" (kernel_stack_ptr));
    kwrite_int(kernel_stack_ptr);
    kwrite_string("\n"); 

    for(;;) {
        asm("hlt");
    } 

}

// Спавн программы
void program_spawn(uint32_t entry_in_ram) {
    uint32_t entry = malloc_page();             
    uint32_t stack_top = malloc_stack()+0x2000;
    kwrite_string("Entry: ");
    kwrite_int(entry);
    kwrite_string("\n");
    kwrite_string("Stack: ");
    kwrite_int(stack_top);
    kwrite_string("\n"); 
    memcpy((int*)entry_in_ram, (int*)entry, 4096);
    process_spawn(entry, stack_top);
    program_execute(entry, stack_top);
    kwrite_string("end"); 
}

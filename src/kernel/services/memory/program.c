#include "../services.h"

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
    );


exit_ptr:
    asm volatile("movl %%esp, %0":"=r" (kernel_stack_ptr));

    for(;;) {
        asm("hlt");
    } 

}
#include <kernel/services.h>
#include <fs/fs.h>

uint32_t kernel_stack_ptr;
uint32_t kernel_stack_base;
uint32_t kernel_return_ptr;

typedef void (*entry_point)();

void exit();

void program_execute(uint32_t entry, uint32_t stack) {
    kernel_return_ptr = (uint32_t)&exit;
   
    entry_point program = (entry_point)entry;

    asm volatile("movl %%esp, %0":"=r" (kernel_stack_ptr));
    asm volatile("movl %%ebp, %0":"=r" (kernel_stack_base));

    asm volatile (   
        "push %%eax\n"
        "push %%ebx\n"
        "push %%ecx\n"
        "push %%edx\n"
        "push %%edi\n"
        "push %%esi\n"
        "movl %1, %%esp\n"   
        "movl %1, %%ebp\n"
        "jmp *%0"               
        : 
        : "r" (program), "r" (stack)
    );
}

void exit() {
    asm volatile(
        "sti\n"
        "pop %%esi\n"
        "pop %%edi\n"
        "pop %%edx\n"
        "pop %%ecx\n"
        "pop %%ebx\n"
        "pop %%eax\n"
        "mov $0x10, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        :
        :
        : "eax", "ebx", "ecx", "edx", "esi", "edi"
    );

    fs->open("SHELL   BIN");
    for(;;) {
        asm("hlt");
    }
}
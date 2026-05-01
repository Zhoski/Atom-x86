#include "syscall.h"

void syscall_handler(int eax, int ebx,int ecx, int edx) {
    /*
     * kwrite_string("---SYSCALL---");
     * kwrite_string("\nEAX  (num): ");
     * kwrite_int(eax);
     * kwrite_string("\nEBX (arg1): ");
     * kwrite_int(ebx);
     * kwrite_string("\nECX (arg2): ");
     * kwrite_int(ecx);
     * kwrite_string("\nEDX (arg3): ");
     * kwrite_int(edx);
     * kwrite_string("\n"); 
    */
    switch(eax) {
        case SYSCALL_WRITE:  
            switch(ebx) {
                case WRITE_TEXT:
                    kwrite_string((uint8_t*)ecx);
                    break;
                case WRITE_INT:
                    kwrite_int(ecx);
                    break;
                case WRITE_CHAR:
                    putchar((uint8_t)ecx);
                defualt:
                    break;
            } 
            asm("movl $0, %%eax":::"%eax");
            break;
        case SYSCALL_KEYBOARD:
            switch(ebx) {
                case RETURN_LAST_SYM:
                    char c = keyboard_buf_get_las_sym();
                    asm("movl %0, %%eax\n"
                        :
                        : "r" ((int)c)
                        : "%eax");

                    break;
                default:
                    break;
            }
            break;
        case SYSCALL_DIED:
            kwrite_string("\nKernel stack before process died: ");

            uint32_t cur_esp;
            asm volatile("movl %%esp, %0":"=r" (cur_esp));
            kwrite_int(cur_esp);
            kwrite_string("\n");
            asm volatile (
                "movl %1, %%esp\n"
                "jmp *%0"
                :
                : "r"(kernel_return_ptr), "r"(kernel_stack_ptr)
                : "esp"
            );    

            break;
        default:
            break;
    } 
}

#include "syscall.h"

void syscall_handler(int eax, int ebx,int ecx, int edx) { 
    switch(eax) {
        case SYSCALL_WRITE:  
            switch(ebx) {
                case WRITE_TEXT:
                    service.vga->write_string((uint8_t*)ecx);
                    break;
                case WRITE_INT:
                    service.vga->write_int(ecx);
                    break;
                case WRITE_CHAR:
                    service.vga->write_char((uint8_t)ecx);
                    break;
                case WRITE_HEX:
                    service.vga->write_hex(ecx, edx);
                defualt:
                    break;
            } 
            asm("movl $0, %%eax":::"%eax");
            break;
        case SYSCALL_KEYBOARD:
            switch(ebx) {
                case RETURN_LAST_SYM:
                    //char c = keyboard_buf_get_las_sym();
                    char c = kb_device.key->get_last_key();
                    asm("movl %0, %%eax\n"
                        :
                        : "r" ((int)c)
                        : "%eax"
                    );

                    break;
                default:
                    break;
            }
            break;
        case SYSCALL_VGA:
            switch(ebx) {
                case SET_ATTRIBUTE:
                    uint8_t bg = ecx >> 8;
                    uint8_t fg = ecx;
                    service.vga->set_attribute(bg, fg);
                    break;
                case CLEAR_SCREEN:
                    service.vga->clear();
                    break;
            }
            break;
        case SYSCALL_DIED:
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
        case SYSCALL_MEMORY:
            switch(ebx) {
                case READ_MEMORY:
                    uint8_t data = service.memory->memread((uint8_t*)ecx);
                    asm volatile(
                        "movl %0, %%eax"
                        :
                        : "r" ((int)data)
                        : "eax"
                    );
                    
                    break;
            }
    } 
}

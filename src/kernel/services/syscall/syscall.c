#include "syscall.h"

void syscall_handler(int eax, int ebx,int ecx, int edx) { 
    switch(eax) {
        case SYSCALL_WRITE:  
            switch(ebx) {
                uint8_t color;
                case WRITE_TEXT:
                    video.write_string((uint8_t*)ecx); 
                    break; 
                case WRITE_CHAR: 
                    video.write_char((uint8_t)ecx);
                    break;
                default:
                    break;
            } 
            asm("movl $0, %%eax":::"%eax");
            break;
        case SYSCALL_KEYBOARD:
            switch(ebx) {
                case RETURN_LAST_SYM:
                    //char c = keyboard_buf_get_las_sym();
                    char c = kb.get_last_key();
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
            uint8_t color;
            switch(ebx) {
                case SET_BG_COLOR:
                    color = (uint8_t)edx;

                    video.terminal_bg_vbe_set(color);
                    
                    break;
                case SET_FG_COLOR:
                    uint8_t color = (uint8_t)edx;
                    video.terminal_fg_vbe_set(color);

                    break;
                case CLEAR_SCREEN:
                    video.clear_screen((uint8_t)edx);
                    break;
            }
            break;
        case SYSCALL_DISK:
            switch (ebx)
            {
                case CAT_FILE:
                    uint8_t *file = (uint8_t*)ecx;
                    uint8_t *out = (uint8_t*)edx;
                    uint32_t status = read_file(file, out);

                    asm volatile(
                        "movl %%eax, %0"
                        : "=r" (status)
                        :
                        : "memory"
                    );
                    break;
            break;
        case SYSCALL_DIED:
            asm volatile (
                "movl %1, %%esp\n"
                "jmp *%0"
                :
                : "r"(kernel_return_ptr), "r"(kernel_stack_ptr)
                : 
            );    
            break;
        default:
            break;
        case SYSCALL_MEMORY:
            switch(ebx) {
                uint8_t data;
                uint32_t target_mem;
                case READ_MEMORY_B:
                    data = service.memory->memread((uint8_t*)ecx);
                    asm volatile(
                        "movl %0, %%eax"
                        :
                        : "r" ((int)data)
                        : "eax"
                    );
                    break;
                case READ_MEMORY_DW:
                    data = service.memory->memread_dw((uint8_t*)ecx);
                    asm volatile(
                        "movl %0, %%eax"
                        :
                        : "r" ((int)data)
                        : "eax"
                    );
                    break;
                case READ_MEMORY_DD:
                    data = service.memory->memread_dd((uint8_t*)ecx);
                    asm volatile(
                        "movl %0, %%eax"
                        :
                        : "r" ((int)data)
                        : "eax"
                    );
                    break;
                case WRITE_MEMORY:
                    data = (uint8_t*)ecx;
                    target_mem = edx;
                    asm volatile(
                        "movl %[data], %[mem]"
                        : [data] "=m" (data)
                        : [mem]  "r"  (target_mem)
                        : "memory"
                    );
                    break;
            }
        }
    } 
}
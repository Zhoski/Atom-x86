#include "syscall.h"

void syscall_handler(int eax, int ebx,int ecx, int edx) { 
    switch(eax) {
        case SYSCALL_WRITE:  
            switch(ebx) {
                uint8_t r, g, b;
                case WRITE_TEXT:
                    r = (uint8_t)(edx >> 24);
                    g = (uint8_t)(edx >> 16);
                    b = (uint8_t)(edx >> 8);
                    video.write_string((uint8_t*)ecx); 
                    break; 
                case WRITE_CHAR: 
                    r = (uint8_t)(edx >> 24);
                    g = (uint8_t)(edx >> 16);
                    b = (uint8_t)(edx >> 8);
                    video.write_char((uint8_t)ecx);
                    break;
                case PUTPIXEL:
                    uint16_t x = (uint16_t)(ecx >> 16);
                    uint16_t y = (uint16_t)ecx;
                    r = (uint8_t)(edx >> 24);
                    g = (uint8_t)(edx >> 16);
                    b = (uint8_t)(edx >> 8);
                    
                    video.putpixel(x,y,r,g,b);
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
        case SYSCALL_VBE:
            uint32_t r;
            uint32_t g;
            uint32_t b;
            switch(ebx) {
                case SET_BG_COLOR:
                    r = (uint8_t)(ecx >> 24);
                    g = (uint8_t)(ecx >> 16);
                    b = (uint8_t)(ecx >> 8);

                    video.terminal_bg_vbe_set(r,g,b);
                    
                    break;
                case SET_FG_COLOR:
                    r = (uint8_t)(ecx >> 24);
                    g = (uint8_t)(ecx >> 16);
                    b = (uint8_t)(ecx >> 8);

                    video.terminal_fg_vbe_set(r,g,b);

                    break;
                case CLEAR_SCREEN:
                    r = (uint8_t)(ecx >> 24);
                    g = (uint8_t)(ecx >> 16);
                    b = (uint8_t)(ecx >> 8);

                    video.clear_screen(r,g,b);
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
}
#include "syscall.h"

extern uint32_t kernel_stack_base;

void syscall_handler(int eax, int ebx,int ecx, int edx) { 
    switch(eax) {
        case SYSCALL_WRITE:  
            switch(ebx) {
                case WRITE_TEXT:
                    video->write_string((uint8_t*)ecx); 
                    break; 
                case WRITE_CHAR: 
                    video->write_char((uint8_t)ecx);
                    break;
                default:
                    break;
            } 
            asm("movl $0, %%eax":::"%eax");
            break;
        case SYSCALL_KEYBOARD:
            switch(ebx) {
                case RETURN_LAST_SYM:
                    char out = kb.get_last_key();
                    eax = out;

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

                    video->terminal_bg_vbe_set(color);
                    
                    break;
                case SET_FG_COLOR:
                    uint8_t color = (uint8_t)edx;
                    video->terminal_fg_vbe_set(color);

                    break;
                case CLEAR_SCREEN:
                    video->clear_screen((uint8_t)edx);
                    break;
                case SET_CURSOR:
                    video->terminal_set_cursor_position((uint8_t)(ecx >> 16), (uint8_t)ecx);
                    break;
                case GET_CURSOR:
                    video->terminal_get_cursor_position(ecx, edx);
                    break;
            }
            break;
        case SYSCALL_DISK:
            switch (ebx)
            {
                case CAT_FILE:
                    eax = fs->read(ecx, edx);
                    break;
                case GET_ROOT:
                    fs->get_root(ecx);
                    break;
            }
            break;
        case SYSCALL_DIED:
            asm volatile (
                "movl %[stack_top], %%esp\n"
                "movl %[stack_base], %%ebp\n"
                "jmp *%[exit]"
                :
                : [exit] "r"(kernel_return_ptr), [stack_top] "r"(kernel_stack_ptr), [stack_base] "r" (kernel_stack_base)
            );    
            break;
        case SYSCALL_MEMORY:
            switch (ebx)
            {
            case MALLOC:
                uint32_t p = service.memory->malloc(ecx);
                eax = p;
                break;
            case FREE:
                service.memory->free(ecx);
                break;
            }
            break;
        case SYSCALL_SYSTEM:
            switch (ebx)
            {
            case SYS_RUN:
                asm("sti");
                fs->open(ecx);
                break;
            }
            break;
    } 
}
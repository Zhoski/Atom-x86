#include <kernel/syscall.h>
#include <kernel/memory.h>
#include <kernel/device.h>
#include <kernel/memory.h>
#include <kernel/program.h>
#include <drivers/video/video.h>
#include <fs/fs.h>
#include <cpu/registers.h>
#include <lib/string.h>

extern uint32_t kernel_stack_base;

void syscall_handler(int eax, int ebx,int ecx, int edx, char* esi, char* edi) { 
    __asm__ volatile("sti"); 
    switch(eax) {
        case SYSCALL_VIDEO:
            switch(ebx) {
                case WRITE_STRING:
                    video->kprintf("%s", (u8*)ecx); 
                    break; 
                case WRITE_CHAR: 
                    video->kputc((uint8_t)ecx);
                    break;
                case WRITE_INT:
                    video->kprintf("%d",ecx);
                    break;
                case WRITE_HEX:
                    video->kprintf("%x", ecx);
                    break;
                case SET_BG_COLOR:
                    video->terminal_bg_vbe_set(ecx);
                    break;
                case SET_FG_COLOR:
                    video->terminal_fg_vbe_set(ecx);
                    break;
                case CLEAR_SCREEN:
                    video->clear_screen(ecx);
                    break;
                case SET_CURSOR:
                    video->terminal_set_cursor_position((u16)(ecx >> 16) & 0xFF, (u16)ecx);
                    break;
                case GET_CURSOR:
                    video->terminal_get_cursor_position(ecx, edx);
                    break;
                default:
                    break;
            } 
            break;
        case SYSCALL_KEYBOARD: 
            switch(ebx) {
                case RETURN_LAST_SYM:
                    eax = kb.get_last_key();

                    break;
                default:
                    break;
            }
            break;
        case SYSCALL_STRING:
            switch (ebx)
            {
            case STRCMP:
                eax = strcmp(esi, edi);
                break;
            case STRLEN:
                eax = strlen(esi);
                break;
            default:
                break;
            }
        case SYSCALL_MEMORY:
            switch (ebx)
            {
            case MALLOC:
                uint32_t p = (u32)malloc(ecx);
                eax = p;
                break;
            case FREE:
                free(esi);
                break;
            }
            break;
        case SYSCALL_DISK:
            switch (ebx) {
                case CAT_FILE:
                    eax = fs->read(esi, (u8*)edx, ecx);
                    break;
                case OVERWRITE_FILE:
                    eax = fs->update(esi, edi, ecx);
                    break;
                case CREATE_FILE:
                    eax = fs->create(esi, ecx);
                    break;
                case DELETE_FILE:
                    eax = fs->delete(esi);
                    break;
                case FILE_CHECK:
                    eax = (u32)(fs->check(esi));
                    break;
                case WRITE_DIR:
                    eax = fs->write_dir(esi);
                    break;

                /**
                 *  FIX: Добавить fopen
                 */
            }
            break;
        case SYSCALL_SYSTEM:
            switch (ebx)
            {
            case SYS_RUN:
                fs->open(esi);
                break;
            case SYS_DIED:
                asm volatile (
                    "movl %[stack_top], %%esp\n"
                    "movl %[stack_base], %%ebp\n"
                    "jmp *%[exit]"
                    :
                    : [exit] "r"(kernel_return_ptr), [stack_top] "r"(kernel_stack_ptr), [stack_base] "r" (kernel_stack_base)
                );
            default:
                break;
            }
    } 

    asm volatile(
        "movl %%eax, %0"
        :
        : "a" (eax)
    );
}
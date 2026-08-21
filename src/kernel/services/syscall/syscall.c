#include <kernel/syscall.h>
#include <kernel/memory.h>
#include <kernel/device.h>
#include <kernel/services.h>
#include <drivers/video/video.h>
#include <fs/fs.h>
#include <cpu/registers.h>
#include <kernel/program.h>

extern uint32_t kernel_stack_base;

void syscall_handler(int eax, int ebx,int ecx, int edx, char* esi, char* edi) { 
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
            uint32_t i = 0;
            uint32_t j = 0;
            uint8_t  file[11];

            while (esi[j] != '\0' && esi[j] != '.')
            {
                if(esi[j] >= 97) {
                    file[i] = esi[j] - 32;
                }else {
                    file[i] = esi[j];
                }
                i++;
                j++;
            }

            while (i < 8)
            {
                file[i++] = ' ';
            }

            while (i < 11)
            {
                j++;
                if(esi[j] >= 97) {
                    file[i] = esi[j] - 32;
                }else {
                    file[i] = esi[j];
                }
                i++;
            }

            switch (ebx)
            {
                case CAT_FILE:
                    eax = fs->read(file, ecx, edx);
                    asm volatile(
                        "movl %%eax, %0"
                        :
                        : "a" (eax)
                    );
                    break;
                case GET_ROOT:
                    fs->get_root(ecx);
                    break;
                case WRITE_FILE: {
                    eax = fs->update(file, edi, ecx);
                    asm volatile(
                        "movl %%eax, %0"
                        :
                        : "a" (eax)
                    );
                    break;
                }
                case CREATE_FILE:
                    fs->create(file, ecx);
                    break;
                case DELETE_FILE:
                    eax = fs->delete(file);
                    asm volatile(
                        "movl %%eax, %0"
                        :
                        : "a" (eax)
                    );
                    break;
                case FILE_CHECK:
                    eax = fs->check(file);
                    asm volatile(
                        "movl %%eax, %0"
                        :
                        : "a" (eax)
                    );
                    break;
                case FILE_RUN:
                    asm("sti");
                    eax = fs->open(file);
                    asm volatile(
                        "movl %%eax, %0"
                        :
                        : "a" (eax)
                    );
                    break;
                case FOPEN:
                {
                    typedef struct __attribute__((packed)) {
                        U8 name[8];
                        U8 ext[3];
                        U16 start_sec;
                        U16 size;
                        U8 flags;
                    } File;

                    typedef struct __attribute__((packed)) {
                        unsigned char name[11];
                        unsigned int bytes;
                        unsigned char* cur;
                        unsigned char* base;
                    } Ret;

                    File* f = (File*)fs->check(file);
                    if(!f) {
                        asm volatile(
                        "movl $0, %%eax"
                            :
                            :
                            : "%eax"
                        );
                        break;
                    }

                    Ret* ret = service.memory->malloc(sizeof(Ret) + f->size);
                    service.memory->memset(ret, 0, sizeof(Ret) + f->size);
                    ret->bytes = f->size;
                    ret->base = (U8*)ret + sizeof(Ret);
                    ret->cur = ret->base;

                    fs->read(file, f->size, ret->base);
                    
                    asm volatile(
                        "movl %0, %%eax"
                        :
                        : "r" (ret)
                        : "%eax"
                    );
                    break;
                }
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
                service.memory->free(esi);
                break;
            }
            break;
    } 
}

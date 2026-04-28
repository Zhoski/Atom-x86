#include "syscall.h"

void syscall_handler(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {

    kwrite_int(eax);
    switch(eax) {
        case SYSCALL_WRITE:
            kwrite_string("SYSCALL_WRITE ");
            break;
        default:
            break;
   }

   kwrite_string("int 0x80");
}

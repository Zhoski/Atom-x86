#ifndef __SYSCALL__
#define __SYSCALL__
#include "../../../drivers/vga.h"
#include "../../../drivers/keyboard.h"

#define SYSCALL_WRITE       1
#define SYSCALL_KEYBOARD    2

extern void syscall_handler(int eax, int ebx,int ecx, int edx);
#endif

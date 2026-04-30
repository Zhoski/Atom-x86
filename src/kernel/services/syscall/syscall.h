#ifndef __SYSCALL__
#define __SYSCALL__
#include "../../../drivers/vga.h"
#include "../../../drivers/keyboard.h"

/*-------- Вывод --------*/
#define SYSCALL_WRITE       1
/*----- Виды вывода -----*/
#define WRITE_TEXT          1
#define WRITE_INT           2

/*----- Клавиатура ------*/
#define SYSCALL_KEYBOARD    2

extern void syscall_handler(int eax, int ebx,int ecx, int edx);
#endif

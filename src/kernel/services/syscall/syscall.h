#ifndef __SYSCALL__
#define __SYSCALL__
#include "../../../drivers/vga.h"
#include "../../../drivers/keyboard.h"
#include "../Memory/memory_map.h"

/*-------- Вывод --------*/
#define SYSCALL_WRITE       1
/*----- Виды вывода -----*/
#define WRITE_TEXT          1
#define WRITE_INT           2
#define WRITE_CHAR          3

/*----- Клавиатура ------*/
#define SYSCALL_KEYBOARD    2
/*--- Вернуть символ ----*/
#define RETURN_LAST_SYM     1

/*----- Завершение ------*/
#define SYSCALL_DIED        60

extern void syscall_handler(int eax, int ebx,int ecx, int edx);
#endif

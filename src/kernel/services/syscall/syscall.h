#ifndef __SYSCALL__
#define __SYSCALL__
#include "../memory/memory_map.h"
#include "../memory/memory.h"
#include "../../device/device.h"
#include "../../services/services.h"

/*-------- Вывод --------*/
#define SYSCALL_WRITE           1
/*----- Виды вывода -----*/
#define WRITE_TEXT              1
#define WRITE_INT               2
#define WRITE_CHAR              3
#define WRITE_HEX               4

/*----- Клавиатура ------*/
#define SYSCALL_KEYBOARD        2
/*--- Вернуть символ ----*/
#define RETURN_LAST_SYM         1

/* -------- VGA ---------*/
#define SYSCALL_VGA             3
#define SET_ATTRIBUTE           1
#define SET_CURSOR_POSITION     2
#define CLEAR_SCREEN            3

/*-------- MEMORY -------*/
#define SYSCALL_MEMORY          4
#define READ_MEMORY             1
#define WRITE_MEMORY            2

/*----- Завершение ------*/
#define SYSCALL_DIED            60

#define KERNEL_READ_ERROR       -1
#define SUCCES_READ             0
#define KERNEL_BASE             0x110000

extern void syscall_handler(int eax, int ebx,int ecx, int edx);
#endif

#ifndef __SYSCALL__
#define __SYSCALL__
#include "../memory/memory_map.h"
#include "../memory/memory.h"
#include "../../device/device.h"
#include "../../services/services.h"
#include "../../../drivers/video/video.h"
#include "../../../drivers/fs/fs.h"
#include "../../../cpu/registers.h"
#include "../memory/program.h"

/*-------- Вывод --------*/
#define SYSCALL_WRITE           1
/*----- Виды вывода -----*/
#define WRITE_TEXT              1
#define WRITE_CHAR              2
#define PUTPIXEL                3

/*----- Клавиатура ------*/
#define SYSCALL_KEYBOARD        2
/*--- Вернуть символ ----*/
#define RETURN_LAST_SYM         1

/*------ GRAPHICS -------*/
#define SYSCALL_VGA             3
#define SET_FG_COLOR            1
#define SET_BG_COLOR            2
#define CLEAR_SCREEN            3

/* ------- MEMORY -------*/
#define SYSCALL_MEMORY          4
#define READ_MEMORY_B           1
#define READ_MEMORY_DW          2
#define READ_MEMORY_DD          3
#define WRITE_MEMORY            4
#define COPY_MEMORY             5
#define MALLOC                  6
#define FREE                    7

/*-------- DISK --------*/      
#define SYSCALL_DISK            5
#define CAT_FILE                1
#define GET_ROOT                2

/*------- SYSTEM -------*/
#define SYSCALL_SYSTEM          6
#define SYS_RUN                 1

/*----- Завершение ------*/
#define SYSCALL_DIED            60

#define KERNEL_READ_ERROR       -1
#define SUCCES_READ             0
#define KERNEL_BASE             0x110000

void syscall_handler(int eax, int ebx,int ecx, int edx);
#endif

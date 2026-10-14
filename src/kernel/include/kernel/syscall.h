#ifndef __SYSCALL__
#define __SYSCALL__

/* VIDEO */

#define SYSCALL_VIDEO           0x01

#define WRITE_STRING            0x01
#define WRITE_CHAR              0x02
#define WRITE_INT               0x03
#define WRITE_HEX               0x04
#define SET_BG_COLOR            0x05
#define SET_FG_COLOR            0x06
#define CLEAR_SCREEN            0x07
#define SET_CURSOR              0x08
#define GET_CURSOR              0x09

/* ----- Keyboard ------ */

#define SYSCALL_KEYBOARD        0x02
#define RETURN_LAST_SYM         0x01

/* ------- STRING ------ */

#define SYSCALL_STRING          0x03

#define STRCMP                  0x01
#define STRLEN                  0x02              

/* ------- MEMORY -------*/

#define SYSCALL_MEMORY          0x04

#define MALLOC                  0x01
#define FREE                    0x02

/*-------- DISK --------*/      

#define SYSCALL_DISK            0x05
#define CAT_FILE                0x01
#define OVERWRITE_FILE          0x02
#define CREATE_FILE             0x03
#define DELETE_FILE             0x04
#define FILE_CHECK              0x05
#define WRITE_DIR               0x06
#define DISK_DISPATCH           0x07

/*------- SYSTEM -------*/
#define SYSCALL_SYSTEM          0x06
#define SYS_RUN                 0x01
#define SYS_DIED                0x60

#define KERNEL_READ_ERROR       -1
#define SUCCES_READ             0
#define KERNEL_BASE             0x110000

void syscall_handler(int eax, int ebx,int ecx, int edx, char* esi, char* edi);
#endif

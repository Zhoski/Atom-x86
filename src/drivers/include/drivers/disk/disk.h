#ifndef __DISK__
#define __DISK__

#include <stdint.h>
#include <lib/int.h>

#define SUCCES_INIT_DISK           0
#define DISK_NOT_FOUND             1
#define DISK_DONT_SUPPORT_PATA     2
#define DISK_ERROR                 3
#define DISK_TIMEOUT               4

typedef struct
{
    U32 (*init)(U16 disk_info[256]);
    U8 (*read_sector)(U32 lba, U16 word[256]);
    U8 (*write_sector)(U32 lba, U16 word[256]);
} Disk;

extern Disk* disk;

U8 disk_init(U16 disk_info[256]);

#endif
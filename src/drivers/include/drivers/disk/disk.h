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
    u32 (*init)(u16 disk_info[256]);
    u8 (*read_sector)(u64 lba, u16 word[256]);
    u8 (*write_sector)(u64 lba, u16 word[256]);
} Disk;

extern Disk* disk;

u8 disk_init(u16 disk_info[256]);

#endif
#ifndef __DISK__
#define __DISK__

#include <stdint.h>
#include "../../kernel/int.h"

typedef struct
{
    void (*init)(U16 disk_info[256]);
    U8 (*read_sector)(U32 lba, U16 word[256]);
    U8 (*write_sector)(U32 lba, U16 word[256]);
} Disk;

extern Disk* disk;

U8 disk_init(U16 disk_info[256]);

#endif
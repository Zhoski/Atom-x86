#ifndef __DISK__
#define __DISK__

#include <stdint.h>

typedef struct
{
    void (*init)(uint16_t disk_info[256]);
    uint8_t (*read_sector)(uint32_t lba, uint16_t word[256]);
    uint8_t (*write_sector)(uint32_t lba, uint16_t word[256]);
} Disk;

extern Disk* disk;

uint8_t disk_init(uint16_t disk_info[256]);

#endif
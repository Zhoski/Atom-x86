#ifndef __DISK__
#define __DISK__

#include <stdint.h>
#include <lib/int.h>

#define SUCCES_INIT_DISK           0
#define DISK_NOT_FOUND             1
#define DISK_DONT_SUPPORT_PATA     2
#define DISK_ERROR                 3
#define DISK_TIMEOUT               4

#define IO_READ                  0x01
#define IO_WRITE                 0x02
#define IO_IDENTIFY              0x03
#define IO_INIT                  0x04

typedef struct io_disk_packet io_disk_packet_t;

struct io_disk_packet {
    u32 command;

    u64 lba_start;
    u16 sec_count;

    void* buffer;
    void* (*callback)(io_disk_packet_t* packet);
    u32 result;

    u32 rsv0[2];
} __attribute__((packed));

typedef struct
{
    void (*dispather)(io_disk_packet_t* packet);
} Disk;

extern Disk* disk;

u8 disk_init(u16 disk_info[256]);

#endif
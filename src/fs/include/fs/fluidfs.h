#ifndef FLUIDFS_H
#define FLUIDFS_H

#include <lib/int.h>

typedef struct node {
    u8 name[32];
    u8 ext[8];
    u8 flag;
    u16 rsv0;
    u32 size;
    u32 rsv1[2];
    u32 p_block[18];
} node_t;

typedef struct super_block {
    u8  sig[4];
    u32 disk_size;
    u32 block_size;
    u32 total_block;
    u32 block_for_bitmap;
    u32 block_root_start;
    u32 sec_p_block;
    u8  rsv[4068];
} super_block_t;


u32 fluidfs_init();
u32 fluidfs_fcheck(u8* file);

extern node_t node;

#endif
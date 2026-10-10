#include <fs/fluidfs.h>
#include <drivers/disk/disk.h>
#include <kernel/memory.h>
#include <drivers/video/video.h>
#include <lib/string.h>

super_block_t* super_block;
node_t* root_node;
node_t node;
u8* cur_path;

void raw_filename_to_std_format(u8* raw_name) {
    video->kprintf("RAW: %s\n", raw_name);
    u32 j = 0, n = 0, k = 0;
    u32 last_slash = 0;

    memset((void*)(&node.name), 0, 32);
    memset((void*)(&node.ext), 0, 8);

    if(cur_path) {
        free(cur_path);
    }

    while (k < 32 && raw_name[k])
    { 
        if(raw_name[k] == '/') {
            last_slash = k;
        }
        k++;
    }

    cur_path = malloc(last_slash + 2);
    memcpy(raw_name, cur_path, last_slash + 1);
    cur_path[last_slash + 1] = '\0';

    if(cur_path[0] != '/') cur_path[0] = '/';

    raw_name = (u8*)(raw_name + last_slash);
    if(*raw_name == '/') {
        raw_name++;
    }

    while (j < 32 && raw_name[j] && raw_name[j] != '.')
    {
        node.name[j] = raw_name[j];
        j++;
    }

    j++;
    while (j < 32 && raw_name[j])
    {
        node.ext[n] = raw_name[j];
        j++;
        n++;
    }
}

u32 fluidfs_init() {
    u8* super_block_buffer = malloc(4096);
    super_block = super_block_buffer;
    root_node = (node_t*)(super_block_buffer + 128);

    io_disk_packet_t packet = {
        .command = IO_READ,
        .lba_start = 8,
        .sec_count = 1,
        .buffer = super_block_buffer,
    };

    disk->dispather(&packet);
}

/**
 *  Возвращает 1 если файл найден и 0 в ином случаи
 */
u32 fluidfs_fcheck(u8* file) {
    raw_filename_to_std_format(file);
    u8* block_buffer = malloc(super_block->block_size);

    io_disk_packet_t packet = {
        .command = IO_READ,
        .sec_count = super_block->sec_p_block,
        .buffer = block_buffer,
    };

    video->kprintf("Search: %s.%s in %s\n", node.name, node.ext, cur_path);
}
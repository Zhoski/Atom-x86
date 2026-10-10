#include <fs/fluidfs.h>
#include <drivers/disk/disk.h>
#include <kernel/memory.h>
#include <kernel/program.h>
#include <drivers/video/video.h>
#include <lib/string.h>

super_block_t* super_block;
node_t* root_node;
node_t node;
u8* path;
u32 slash_count = 0;

u32 node_p_block;

void raw_filename_to_std_format(u8* raw_name) {
    u32 j = 0, n = 0, k = 0;
    u32 last_slash = 0;
    slash_count = 0;

    memset((void*)(&node.name), 0, 32);
    memset((void*)(&node.ext), 0, 8);

    if(path) {
        free(path);
    }

    while (k < 32 && raw_name[k])
    { 
        if(raw_name[k] == '/') {
            last_slash = k;
        }
        k++;
    }

    path = malloc(last_slash + 2);
    memcpy(raw_name, path, last_slash + 1);
    path[last_slash + 1] = '\0';

    if(path[0] != '/') path[0] = '/';

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

    u32 i = 0;
    while (path[i])
    {
        if(path[i] == '/') {
            path[i] = '\0';
            slash_count++;
        }
        i++;
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

    node_p_block = super_block->block_size / sizeof(node);
}

/**
 *  Возвращает 1 если файл найден и 0 в ином случаи
 */
u32 fluidfs_fcheck(u8* file) {
    raw_filename_to_std_format(file);
    u8* block_buffer = malloc(super_block->block_size);

    io_disk_packet_t packet0 = {
        .command = IO_READ,
        .sec_count = super_block->sec_p_block,
        .buffer = block_buffer,
    };

    node_t* cur_node = block_buffer;
    node_t* cur_node_dir = root_node;
    u32 path_offset = 1;
    u32 dir_found = 0;

    for(u32 k = 0; k < slash_count - 1; k++) {
        dir_found = 0;
        for(u32 i = 0; i < 16; i++) {
            if(dir_found) {
                break;
            }

            packet0.lba_start = cur_node_dir->p_block[i] * super_block->sec_p_block;

            disk->dispather(&packet0);

            cur_node = block_buffer;

            if(!cur_node->name[0]) {
                free(block_buffer);
                return 0;
            }

            for(u32 j = 0; j < node_p_block; j++) {
                //video->kprintf("step: %d    cmp %s and %s\n", j, path + path_offset, cur_node->name);
                if(strcmp(path + path_offset, cur_node->name) == 0 && cur_node->flag == 0xFF) {
                    path_offset += strlen(path + path_offset) + 1;
                    memcpy(cur_node, cur_node_dir, 128);
                    i = 0;
                    dir_found = 1;
                    break;
                }

                cur_node = (node_t*)((u8*)cur_node + 128);

                if(cur_node->name[0] == 0) {
                    break;
                }
            }
        }

        if(!dir_found) {
            free(block_buffer);
            return 0;
        }
    }

    for(u32 i = 0; i < 16; i++) {
        packet0.lba_start = cur_node_dir->p_block[i] * super_block->sec_p_block;

        disk->dispather(&packet0);

        cur_node = block_buffer;

        if(cur_node->name[0] == 0) {;
            break;
        }

        for(u32 j = 0; j < node_p_block; j++) {
            if((strcmp(node.name, cur_node->name) == 0) && (strcmp(node.ext, cur_node->ext) == 0)) {
                memcpy(cur_node, &node, 128);
                free(block_buffer);
                return 1;
            }

            cur_node = (node_t*)((u8*)cur_node + 128);

            if(cur_node->name[0] == 0) {
                break;
            }
        }
    }

    free(block_buffer);
    return 0;
}

/**
 *  Возвращает 0 если файл не найден
 *  Загружает файл на 0x300000 после чего передает ему управление
 */
u32 fluidfs_fopen(u8* file) {
    video->kprintf("fopen\n");
    if(!fluidfs_fcheck(file)) {
        video->kprintf("beda\n");
        return 0;
    }

    u32 offset = 0;
    u32 stack = 0x400000;
    u32 entry = 0x300000;

    io_disk_packet_t packet0 = {
        .command = IO_READ,
        .buffer = entry,
        .sec_count = super_block->sec_p_block
    };

    for(u32 k = 0; k < 16; k++) {
        if(node.p_block[k] == 0) {
            break;
        }

        packet0.lba_start = node.p_block[k] * super_block->sec_p_block;
        packet0.buffer = entry + offset;

        disk->dispather(&packet0);

        offset += super_block->sec_p_block * 512;
    }

    video->kprintf("test: %x\n", *(u8*)0x300000);

    program_execute(entry, stack);
}
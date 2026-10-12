#include <fs/fluidfs.h>
#include <drivers/disk/disk.h>
#include <kernel/memory.h>
#include <kernel/program.h>
#include <drivers/video/video.h>
#include <lib/string.h>

#define FILE_NOT_FOUND          0x00
#define FILE_FOUND              0x01
#define NODE_CREATED            0x02

#define OUT_OF_FILE_BOUNDS      0x02
#define DISK_OWERFLOW           0x03

super_block_t* super_block;
node_t* root_node;
node_t* g_cur_node;
node_t g_node;
u8* path;
u32 slash_count = 0;

u32 node_p_block;

u8* g_fcheck_buffer;
u32 g_cur_block;
u8* block_bitmap;

u32 get_block() {
    u32 bit = 0;
    
    while (1)
    {
        u32 byte = bit / 8;
        if(byte > 512) {
            return 0;
        }
        u32 bit_in_byte = bit % 8;
        if((block_bitmap[byte] & (1 << bit_in_byte)) == 0) {          
            block_bitmap[byte] |= (1 << bit_in_byte);

            return byte * 8 + bit_in_byte;
        }else {
            bit++;
        }
    }   
}

void free_block(u32 block) {
    if(block == 0) {
        return;
    }

    u32 bit = block;
    
    u32 byte = bit / 8;
    u32 bit_in_byte = bit % 8;
    block_bitmap[byte] &= ~(1 << bit_in_byte);

    //video->kprintf("Block released: %d\n", block);
}

void raw_filename_to_std_format(u8* raw_name) {
    u32 j = 0, n = 0, k = 0;
    u32 last_slash = 0;
    slash_count = 0;

    memset((void*)&g_node, 0, 128);

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
        g_node.name[j] = raw_name[j];
        j++;
    }

    j++;
    while (j < 32 && raw_name[j])
    {
        g_node.ext[n] = raw_name[j];
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
    u8* super_block_buffer = malloc(512);
    block_bitmap = malloc(512);
    super_block = super_block_buffer;
    root_node = (node_t*)(super_block_buffer + 128);

    io_disk_packet_t packet = {
        .command = IO_READ,
        .lba_start = 8,
        .sec_count = 1,
        .buffer = super_block_buffer,
    };

    disk->dispather(&packet);

    packet.buffer = block_bitmap;
    packet.lba_start += super_block->sec_p_block;

    disk->dispather(&packet);

    node_p_block = super_block->block_size / sizeof(g_node);

    g_fcheck_buffer = malloc(super_block->block_size);
}

/**
 *  Возвращает 1 если файл найден и 0 в ином случаи
 */
u32 fluidfs_fcheck(u8* file) {
    raw_filename_to_std_format(file);

    io_disk_packet_t packet0 = {
        .command = IO_READ,
        .sec_count = super_block->sec_p_block,
        .buffer = g_fcheck_buffer,
    };

    node_t* cur_node = g_fcheck_buffer;
    node_t* cur_node_dir = (node_t*)malloc(128);
    u32 path_offset = 1;
    u32 dir_found = 0;

    memcpy(root_node, cur_node_dir, 128);

    for(u32 k = 0; k < slash_count - 1; k++) {
        dir_found = 0;
        for(u32 i = 0; i < 16; i++) {
            if(dir_found) {
                break;
            }

            if(cur_node_dir->p_block[i] == 0) {
                free(cur_node_dir);
                return FILE_NOT_FOUND;
            }

            packet0.lba_start = cur_node_dir->p_block[i] * super_block->sec_p_block;

            disk->dispather(&packet0);

            cur_node = g_fcheck_buffer;

            if(!cur_node->name[0]) {
                free(cur_node_dir);
                return FILE_NOT_FOUND;
            }

            for(u32 j = 0; j < node_p_block; j++) {
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
            free(cur_node_dir);
            return FILE_NOT_FOUND;
        }
    }

    for(u32 i = 0; i < 16; i++) {
        packet0.lba_start = cur_node_dir->p_block[i] * super_block->sec_p_block;

        disk->dispather(&packet0);

        cur_node = g_fcheck_buffer;

        if(cur_node->name[0] == 0) {;
            free(cur_node_dir);
            return 0;
        }

        for(u32 j = 0; j < node_p_block; j++) {
            if((strcmp(g_node.name, cur_node->name) == 0) && (strcmp(g_node.ext, cur_node->ext) == 0)) {
                memcpy(cur_node, &g_node, 128);
                g_cur_node = cur_node;
                g_cur_block = cur_node_dir->p_block[i];
                free(cur_node_dir);
                return FILE_FOUND;
            }

            cur_node = (node_t*)((u8*)cur_node + 128);

            if(cur_node->name[0] == 0) {
                free(cur_node_dir);
                return 0;
            }
        }
    }

    return 0;
}

/**
 *  Возвращает 0 если файл не найден
 *  Загружает файл на 0x300000 после чего передает ему управление
 */
u32 fluidfs_fopen(u8* file) {
    if(!fluidfs_fcheck(file)) {
        return FILE_NOT_FOUND;
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
        if(g_node.p_block[k] == 0) {
            break;
        }

        packet0.lba_start = g_node.p_block[k] * super_block->sec_p_block;
        packet0.buffer = entry + offset;

        disk->dispather(&packet0);

        offset += super_block->sec_p_block * 512;
    }

    video->kprintf("test: %x\n", *(u8*)0x300000);

    program_execute(entry, stack);
}

/**
 *  Читает n количество байт file в out
 */
u32 fluidfs_fread(u8* file, u8* dest, u32 n) {
    if(!fluidfs_fcheck(file)) {
        return FILE_NOT_FOUND;
    }

    if(g_node.size < n) {
        return OUT_OF_FILE_BOUNDS;
    }

    u8* temp_buffer = malloc(super_block->block_size);

    io_disk_packet_t packet0 = {
        .command = IO_READ,
        .sec_count = super_block->sec_p_block,
        .buffer = temp_buffer,
    };

    u32 blocks_p_read = (n + super_block->block_size - 1) / super_block->block_size;

    for(u32 k = 0; k < blocks_p_read; k++) {
        packet0.lba_start = g_node.p_block[k] * super_block->sec_p_block;

        disk->dispather(&packet0);

        if(n > super_block->block_size) {
            memcpy(temp_buffer, dest, super_block->block_size);
            n -= super_block->block_size;
        }else {
            memcpy(temp_buffer, dest, n % super_block->block_size);
            break;
        }
    }

    free(temp_buffer);
}

/**
 *  Записывает в file из src n байт
 */
u32 fluidfs_fwrite(u8* file, u8* src, u32 n) {
    if(!fluidfs_fcheck(file)) {
        return FILE_NOT_FOUND;
    }

    u8* temp_buffer = malloc(super_block->block_size);

    io_disk_packet_t packet0 = {
        .command = IO_WRITE,
        .buffer = temp_buffer,
        .sec_count = super_block->sec_p_block
    };

    io_disk_packet_t packet1 = {
        .command = IO_WRITE,
        .buffer = g_fcheck_buffer,
        .sec_count = super_block->sec_p_block,
        .lba_start = g_cur_block * super_block->sec_p_block
    };

    u32 blocks_p_overwriting = (n + super_block->block_size - 1) / super_block->block_size;
    u32 stop_flag = 0;

    for(u32 k = 0; k < blocks_p_overwriting; k++) {
        if(stop_flag) {
            break;
        }

        if(!g_cur_node->p_block[k]) {
            g_cur_node->p_block[k] = get_block();
            if(!g_cur_node->p_block[k]) {
                video->kprintf("Disk overflow\n");
                return DISK_OWERFLOW;
            }
        }

        packet0.lba_start = g_cur_node->p_block[k] * super_block->sec_p_block;

        if(n >= super_block->block_size) {
            memcpy(src, temp_buffer, super_block->block_size);
            n -= super_block->block_size;
        }else {
            memset(temp_buffer, 0, super_block->block_size);
            memcpy(src, temp_buffer, n % super_block->block_size);
            stop_flag = 1;
        }

        disk->dispather(&packet0);
    }

    memset(temp_buffer, 0, super_block->block_size);

    for(u32 k = blocks_p_overwriting; k < 16; k++) {
        if(g_cur_node->p_block[k] == 0) {
            break;
        }
        packet0.lba_start = g_cur_node->p_block[k] * super_block->sec_p_block;

        disk->dispather(&packet0);

        free_block(g_cur_node->p_block[k]);

        g_cur_node->p_block[k] = 0;
    }

    // Изменение нода файла
    g_cur_node->size = n;

    disk->dispather(&packet1);

    // Изменение битмап карты
    packet0.lba_start = 2 * super_block->sec_p_block;
    packet0.sec_count = 1;
    packet0.buffer = block_bitmap;

    disk->dispather(&packet0);

    free(temp_buffer);
}

/**
 *  Удаляет file
 */
u32 fluidfs_fdel(u8* file) {
    if(!fluidfs_fcheck(file)) {
        return FILE_NOT_FOUND;
    }

    u8* temp_buffer = malloc(super_block->block_size);
    memset(temp_buffer, 0, super_block->block_size);

    io_disk_packet_t packet0 = {
        .command = IO_WRITE,
        .sec_count = super_block->sec_p_block,
        .buffer = g_fcheck_buffer,
        .lba_start = g_cur_block * super_block->sec_p_block,
    };

    io_disk_packet_t packet1 = {
        .command = IO_WRITE,
        .sec_count = 1,
        .buffer = block_bitmap,
        .lba_start = 2 * super_block->sec_p_block,
    };

    g_cur_node->name[0] = 0xFF;

    disk->dispather(&packet0);

    for(u32 k = 0; k < 16; k++) {
        if(g_cur_node->p_block[k] == 0) {
            break;
        }

        packet0.lba_start = g_cur_node->p_block[k] * super_block->sec_p_block;
        packet0.buffer = temp_buffer;
        disk->dispather(&packet0);

        free_block(g_cur_node->p_block[k]);

        g_cur_node->p_block[k] = 0;
    }

    disk->dispather(&packet1);

    free(temp_buffer);
}

/**
 *  Создает ноду node с атрибутом flag
 */
u32 fluidfs_cnode(u8* node, u8 flag) {
    raw_filename_to_std_format(node);

    io_disk_packet_t packet0 = {
        .command = IO_READ,
        .sec_count = super_block->sec_p_block,
        .buffer = g_fcheck_buffer,
    };

    node_t* cur_node = g_fcheck_buffer;
    node_t* cur_node_dir = (node_t*)malloc(128);
    u32 path_offset = 1;
    u32 dir_found = 0;

    memcpy(root_node, cur_node_dir, 128);

    video->kprintf("cur dir: %s\n", cur_node_dir->name);

    for(u32 k = 0; k < slash_count - 1; k++) {
        dir_found = 0;
        for(u32 i = 0; i < 16; i++) {
            if(dir_found) {
                break;
            }

            if(cur_node_dir->p_block[i] == 0) {
                free(cur_node_dir);
                return FILE_NOT_FOUND;
            }

            packet0.lba_start = cur_node_dir->p_block[i] * super_block->sec_p_block;

            disk->dispather(&packet0);

            cur_node = g_fcheck_buffer;

            if(!cur_node->name[0]) {
                free(cur_node_dir);
                return FILE_NOT_FOUND;
            }

            for(u32 j = 0; j < node_p_block; j++) {
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
            free(cur_node_dir);
            return FILE_NOT_FOUND;
        }
    }

    video->kprintf("cur node: %s\n", cur_node_dir->name);
    video->kprintf("cur node block: %d\n", cur_node_dir->p_block[0]);

    for(u32 i = 0; i < 16; i++) {
        packet0.lba_start = cur_node_dir->p_block[i] * super_block->sec_p_block;

        disk->dispather(&packet0);

        cur_node = g_fcheck_buffer;

        /*if(cur_node->name[0] == 0) {;
            free(cur_node_dir);
            return 0;
        }*/

        for(u32 j = 0; j < node_p_block; j++) {
            if(cur_node->name[0] == 0 || cur_node->name[0] == 0xFF) {
                io_disk_packet_t packet1 = {
                    .command = IO_WRITE,
                    .sec_count = super_block->sec_p_block,
                    .buffer = g_fcheck_buffer,
                    .lba_start = cur_node_dir->p_block[i] * super_block->sec_p_block,
                };

                g_node.flag = flag;
                g_node.p_block[0] = get_block();

                memcpy(&g_node, cur_node, 128);

                disk->dispather(&packet1);

                packet1.sec_count = 1;
                packet1.lba_start = 2 * super_block->sec_p_block;
                packet1.buffer = block_bitmap;

                disk->dispather(&packet1);

                free(cur_node_dir);
                return NODE_CREATED;
            }

            cur_node = (node_t*)((u8*)cur_node + 128);
        }
    }
}
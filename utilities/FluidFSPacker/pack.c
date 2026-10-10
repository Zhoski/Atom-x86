#include "include/pack.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#define NODE_SIZE            128
#define RSV_BLOCK           0x02

#define DISK_CREATE_ERROR   0x01

#define NO_BOOT_SELECTED    0x02

typedef struct file_node {
    u8 name[32];
    u8 ext[8];
    u8 flag;
    u16 rsv0;
    u32 size;
    u32 rsv1[2];
    u32 p_block[18];
} file_node_t;

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

u8* disk_buffer;

u32 free_block_get() {
    if(!disk_buffer) {
        return 0;
    }
    u8* bitmap = (u8*)(disk_buffer + 0x2000);
    u32 bit = 0;
    
    while (1)
    {
        u32 byte = bit / 8;
        u32 bit_in_byte = bit % 8;
        if((bitmap[byte] & (1 << bit_in_byte)) == 0) {          
            bitmap[byte] |= (1 << bit_in_byte);


            return byte * 8 + bit_in_byte;
        }else {
            bit++;
        }
    }   
}

void raw_filename_to_std_format(u8* raw, file_node_t* file) {
    u32 j = 0, n = 0, k = 0;
    u32 last_slash = 0;

    while (k < 32 && raw[k])
    { 
        if(raw[k] == '/') {
            last_slash = k;
        }
        k++;
    }

    raw = (u8*)(raw + last_slash);
    if(*raw == '/') {
        raw++;
    }

    while (j < 32 && raw[j] && raw[j] != '.')
    {
        file->name[j] = raw[j];
        j++;
    }

    j++;
    while (j < 32 && raw[j])
    {
        file->ext[n] = raw[j];
        j++;
        n++;
    }
}

void file_read(u8* file, u8* out) {
    FILE* f = fopen(file, "r");
    if(!f) {
        return;
    }

    fseek(f, 0, SEEK_END);
    u32 size = ftell(f);
    fseek(f, 0, SEEK_SET);

    out = malloc(size);
    fread(out, 1, size, f);
    fclose(f);
}

u32 disk_img_create(request_t* req) {
    FILE* disk = fopen(req->disk, "w");

    if(!disk) {
        printf("[ FAIL ] Disk create error\n");
        return DISK_CREATE_ERROR;
    }

    disk_buffer = malloc(req->disk_size);

    super_block_t* super_block = (super_block_t*)(disk_buffer + 0x1000);
    super_block->sig[0] = 'F';
    super_block->sig[1] = 'F';
    super_block->sig[2] = 'S';
    super_block->sig[3] = '1';
    super_block->disk_size = req->disk_size;
    super_block->block_size = req->block_size;
    super_block->total_block = req->disk_size / req->block_size;
    super_block->block_for_bitmap = (super_block->total_block + 32767) / 32768;
    super_block->block_root_start = RSV_BLOCK + super_block->block_for_bitmap;
    super_block->sec_p_block = super_block->block_size / 512;

    file_node_t* root = (file_node_t*)(disk_buffer + 0x1000 + 128);
    
    memcpy(root->name, "/", 1);
    root->flag = 0xFF;
    root->p_block[0] = super_block->block_root_start;

    file_node_t* boot = (file_node_t*)(disk_buffer + (super_block->block_root_start * req->block_size));
    
    memcpy(boot->name, "boot", 4);
    boot->flag = 0xFF;
    boot->p_block[0] = super_block->block_root_start + 1;

    file_node_t* system = (file_node_t*)(disk_buffer + (super_block->block_root_start * req->block_size) + 128);
    
    memcpy(system->name, "system", 6);
    system->flag = 0xFF;
    system->p_block[0] = super_block->block_root_start + 2;

    file_node_t* home = (file_node_t*)(disk_buffer + (super_block->block_root_start * req->block_size) + 256);
    
    memcpy(home->name, "home", 4);
    home->flag = 0xFF;
    home->p_block[0] = super_block->block_root_start + 3;

    u64* block_bitmap = (u64*)(disk_buffer + 0x2000);

    u32 bit = 0;

    printf("BITMAP: %d\n", super_block->block_for_bitmap);

    for(u32 i = 0; i < RSV_BLOCK; i++) {
        *block_bitmap |= (1 << bit);
        bit++;
    }

    for(u32 i = 0; i < super_block->block_for_bitmap; i++) {
        *block_bitmap |= (1 << bit);
    }

    for(u32 i = 0; i < 5; i++) {
        *block_bitmap |= (1 << (bit + i));
    }

    fwrite(disk_buffer, 1, req->disk_size, disk);
    fclose(disk);
    
    printf("FluidFSPacker: created disk: %s\n   size: %d\n   block size: %d\n", req->disk, req->disk_size    , req->block_size);
}

u32 boot_set(request_t* req) {
    if(!req->boot) {
        printf("[ FAIL ] No boot selected\n");
        return NO_BOOT_SELECTED;
    }

    u8 boot_buffer[512];

    FILE* boot = fopen(req->boot, "r");
    if(!boot) {
        return;
    }

    fseek(boot, 0, SEEK_END);
    u32 size = ftell(boot);
    fseek(boot, 0, SEEK_SET);

    fread(boot_buffer, 1, size, boot);
    fclose(boot);

    FILE* disk = fopen(req->disk, "r+b");
    if(!disk) {
        return;
    }

    fseek(disk, 0, SEEK_END);
    u32 disk_size = ftell(disk);
    fseek(disk, 0, SEEK_SET);

    disk_buffer = malloc(disk_size);

    fread(disk_buffer, 1, disk_size, disk);

    memcpy(disk_buffer, boot_buffer, 512);

    fseek(disk, 0, SEEK_SET);
    
    fwrite(disk_buffer, 1, disk_size, disk);

    fclose(disk);

    free(disk_buffer);
}

u32 main_boot_set(request_t* req) {
    if(!req->boot) {
        printf("[ FAIL ] No boot selected\n");
        return NO_BOOT_SELECTED;
    }

    FILE* disk = fopen(req->disk, "r+b");
    if(!disk) {
        return;
    }

    fseek(disk, 0, SEEK_END);
    u32 disk_size = ftell(disk);
    fseek(disk, 0, SEEK_SET);

    disk_buffer = malloc(disk_size);

    fread(disk_buffer, 1, disk_size, disk);

    fseek(disk, 0, SEEK_SET);

    super_block_t* super_block = (disk_buffer + 0x1000);

    file_node_t* boot_dir = (disk_buffer + super_block->block_root_start * 0x1000);

    u8* boot_buffer = (u8*)(disk_buffer + 512);

    FILE* boot = fopen(req->main_boot, "r");
    if(!boot) {
        return;
    }

    fseek(boot, 0, SEEK_END);
    u32 size = ftell(boot);
    fseek(boot, 0, SEEK_SET);

    fread(boot_buffer, 1, size, boot);
    fclose(boot);

    for(u32 i = 0; i < 32; i++) {
        if(strcmp(boot_dir->name, "boot") == 0 && boot_dir->flag == 0xFF) {
            file_node_t* boot = (disk_buffer + boot_dir->p_block[0] * 0x1000);
            for(u32 k = 0; k < 32; k++) {
                if(!boot->name[0]) {
                    break;
                }
                boot += 128;
            }

            raw_filename_to_std_format(req->main_boot, boot);

            boot->flag |= 0x01;
            boot->size = size;

            boot->p_block[0] = free_block_get();

            memcpy((void*)(disk_buffer + (boot->p_block[0] * 0x1000)), (void*)boot_buffer, size);

            break;
        }

        boot_dir += 128;
    }

    fwrite(disk_buffer, 1, disk_size, disk);

    fclose(disk);

    free(disk_buffer);
}

u32 file_push(request_t* req) {
    u32 i = 0;
    u32 slash_count = 0;
    while(req->out_path[i]) {
        if(req->out_path[i] == '/') {
            req->out_path[i] = 0;
            slash_count++;
        }
        i++;
    }

    FILE* disk = fopen(req->disk, "r+b");
    if(!disk) {
        return;
    }

    fseek(disk, 0, SEEK_END);
    u32 disk_size = ftell(disk);
    fseek(disk, 0, SEEK_SET);

    disk_buffer = malloc(disk_size);
    super_block_t* super_block = (super_block_t*)(disk_buffer + 0x1000);

    fread(disk_buffer, 1, disk_size, disk);

    fseek(disk, 0, SEEK_SET);

    for(u32 k = 0; k < req->argc; k++) {
        //printf("cardex32: push %s to %s\n", req->argv[k], req->out_path);

        file_node_t file_node;
        u8* file_buffer;
        raw_filename_to_std_format(req->argv[k], &file_node);

        file_node.flag = 0x01;
        
        FILE* file = fopen(req->argv[k], "r");
        if(!file) {
            printf("[ FAIL ] File { %s } not found\n");
            continue;
        }

        fseek(file, 0, SEEK_END);
        u32 file_size = ftell(file);
        file_node.size = file_size;

        u32 aligned_size = (file_size + super_block->block_size - 1) & ~(super_block->block_size - 1);
        u32 block_count = aligned_size / super_block->block_size;

        fseek(file, 0, SEEK_SET);
        
        file_buffer = malloc(aligned_size);

        fread(file_buffer, 1, aligned_size, file);
        fclose(file);

        u8* bitmap = (u8*)(disk_buffer + 0x2000);
        u32 bit = 0;
        u32 block_index = 0;
        u32 file_offset = 0;

        while (block_count)
        {
            file_node.p_block[block_index] = free_block_get();
            memcpy((disk_buffer + (file_node.p_block[block_index] * super_block->block_size)), (file_buffer + file_offset), super_block->block_size);
            file_offset += super_block->block_size;

            bit++;
            block_count--;
            block_index++;
        }

        u32 offset = 1;
        file_node_t* dir = (file_node_t*)(disk_buffer + super_block->block_root_start * super_block->block_size);
        if(*(req->out_path + offset)) {
            for(u32 i = 0; i < slash_count; i++) {
                for(u32 j = 0; j < 32; j++) {
                    if((strcmp(dir->name, req->out_path + offset) == 0)) {
                        dir = (file_node_t*)(disk_buffer + dir->p_block[0] * super_block->block_size);
                        offset += strlen(req->out_path + offset);
                        break;
                    }
                    else {
                        dir = (file_node_t*)((u8*)dir + 128);
                    }
                }

                if(dir->name[0] == 0) {
                    break;
                }
            }
        }   

        while (*dir->name)
        {
            dir = (file_node_t*)((u8*)dir + 128);
        }

        memcpy(dir, &file_node, 128);
        
    }

    fwrite(disk_buffer, 1, disk_size, disk);

    fclose(disk);
    free(disk_buffer);
}

void pack(request_t* req) {
    if(!req->disk) {
        printf("[ WARN ] No disk selected\n");
        return;
    }

    if(req->flag0 & 0x01) disk_img_create(req);
    if(req->flag0 & 0x02) file_push(req);
    if(req->flag0 & 0x04) boot_set(req);
    if(req->flag0 & 0x08) main_boot_set(req);
}
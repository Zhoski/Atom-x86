#include "afs.h"
#include "../disk/disk.h"
#include "../video/video.h"
#include "../../kernel/device/device.h"
#include "../../kernel/services/services.h"
#include "../../kernel/services/memory/program.h"

#define RECORD_SIZE         16      // byte
#define ROOT_SECTORS        16
#define ROOT_BASE            2

#define TRUE                 1
#define FALSE                0

#define FILE_DELETED       255

#define NO_FREE_ENTRY_FOUND  1
#define FILE_NOT_FOUND       1
#define FILE_FOUND           0
#define SUCCES               0

File _file;
static uint16_t next_free_sector = 0;
static uint16_t file_lba_index = 0;

static inline uint8_t cmpFileName(uint8_t *file_name1, uint8_t *file_name2) {
    uint8_t counter = 0;
    while(counter != 11 && *file_name1 == *file_name2) {
        counter++;
        file_name1++;
        file_name2++;
    }

    return counter == 11;
}

uint8_t afs_init() {
    uint8_t* AFS_ROOT = service.memory->malloc(8192);
    uint8_t* AFS_HEAD = AFS_ROOT;

    for(uint32_t i = 0;i < ROOT_SECTORS;i++) {
        disk->read_sector(ROOT_BASE + i, AFS_ROOT + i * 512);
    }

    File* file = (File*)AFS_HEAD;
    uint16_t file_max_start_sec = file->start_sec;
    uint16_t file_size_in_sec = (file->size + 511) >> 9;

    while (*AFS_HEAD)
    {
        if(file_max_start_sec < file->start_sec) {
            file_max_start_sec = file->start_sec;
            file_size_in_sec = (file->size + 511) >> 9;
        }

        AFS_HEAD += RECORD_SIZE;
        file = (File*)AFS_HEAD;
    }

    service.memory->free(AFS_ROOT);
    next_free_sector = file_size_in_sec + file_max_start_sec;
    return SUCCES;
}

uint8_t find_file(const uint8_t *file_name) {
    uint8_t* AFS_ROOT = service.memory->malloc(8192);
    uint8_t* AFS_HEAD = AFS_ROOT;

    for(uint32_t i = 0;i < ROOT_SECTORS;i++) {
        disk->read_sector(ROOT_BASE + i, AFS_ROOT + (i << 9));  // i << 9 == i * 512
    }

    file_lba_index = 0;

    while (*AFS_HEAD)
    {
        if(*AFS_HEAD == FILE_DELETED) {
            AFS_HEAD += RECORD_SIZE;
            continue;
        }

        if(cmpFileName(AFS_HEAD, file_name)) {
            uint8_t* p_to_file = (uint8_t*)&_file;
            service.memory->memcpy(AFS_HEAD, p_to_file, RECORD_SIZE);

            service.memory->free(AFS_ROOT);
            return FILE_FOUND;
        }

        AFS_HEAD += RECORD_SIZE;
        file_lba_index++;
        if(AFS_HEAD >= (8192 + AFS_ROOT)) {
            break;
        }
    }

    service.memory->free(AFS_ROOT);

    return FILE_NOT_FOUND;
}

uint8_t afs_open(const uint8_t *file_name) {
    uint8_t file = find_file(file_name);
    if(file == FILE_NOT_FOUND)
        return FILE_NOT_FOUND;

    uint32_t size_in_sec = (_file.size + 511) >> 9;

    /* Загрузка файла в память */
    uint16_t buffer[256];
    uint32_t entry = 0x300000;       /* Сюда грузим программу */
    uint32_t offset = 0;
    for(uint32_t i = 0;i < size_in_sec;i++) {
        disk->read_sector(_file.start_sec + i, buffer);
        service.memory->memcpy(buffer, (entry+offset), 512);
        offset += 512;
    }

    //program_spawn(entry);
    uint32_t stack = 0x400000;
    program_execute(entry, stack);
}

uint8_t afs_read(const uint8_t *file_name, uint8_t *out) {
    uint8_t file = find_file(file_name);
    if(file == FILE_NOT_FOUND)
        return FILE_NOT_FOUND;

    uint32_t size_in_sec = (_file.size + 511) >> 9;

    uint8_t *file_buffer = service.memory->malloc(512);
    uint8_t *head_out = out;

    for(uint32_t i = 0;i < size_in_sec;i++) {
        disk->read_sector(_file.start_sec + i, file_buffer);
        service.memory->memcpy(file_buffer, head_out, 512);
        head_out += 512;
    }

    out[_file.size] = '\0';

    service.memory->free(file_buffer);

    return SUCCES;
} 

uint8_t afs_delete(const uint8_t* file_name) {
    uint8_t* AFS_ROOT_BUFFER = service.memory->malloc(512);
    uint8_t* AFS_ROOT_HEAD = AFS_ROOT_BUFFER;

    for(uint32_t sector = ROOT_BASE; sector < ROOT_BASE + ROOT_SECTORS;sector++) { 
        disk->read_sector(sector, AFS_ROOT_BUFFER);
        for(uint32_t i = 0;i < 32;i++) {
            if(cmpFileName(file_name, AFS_ROOT_HEAD)) {
                *AFS_ROOT_HEAD = FILE_DELETED;

                disk->write_sector(sector, AFS_ROOT_BUFFER);

                service.memory->free(AFS_ROOT_BUFFER);
                return SUCCES;
            }
            AFS_ROOT_HEAD += RECORD_SIZE;
        }
        AFS_ROOT_HEAD = AFS_ROOT_BUFFER;  
    }

    service.memory->free(AFS_ROOT_BUFFER);
    return FILE_NOT_FOUND;
}

uint8_t afs_create(const uint8_t* file_name, uint16_t size) {
    uint8_t* AFS_ROOT_BUFFER = service.memory->malloc(512);
    uint8_t* AFS_ROOT_HEAD = AFS_ROOT_BUFFER;

    for(uint32_t sector = ROOT_BASE; sector < ROOT_BASE + ROOT_SECTORS;sector++) {   
        disk->read_sector(sector, AFS_ROOT_BUFFER);

        for(uint32_t i = 0; i < 32;i++) {
            if(!*AFS_ROOT_HEAD || *AFS_ROOT_HEAD == FILE_DELETED) {
                File new_file;

                service.memory->memcpy(file_name, (uint8_t*)&new_file, 11);
                new_file.size = size;
                new_file.start_sec = next_free_sector;
                new_file.flags = 0x1;

                next_free_sector += (size + 511) >> 9;

                service.memory->memcpy((uint8_t*)&new_file, AFS_ROOT_HEAD, 16);

                disk->write_sector(sector, AFS_ROOT_BUFFER);

                service.memory->free(AFS_ROOT_BUFFER);
                
                return SUCCES;
            }

            AFS_ROOT_HEAD += RECORD_SIZE;
        }
        AFS_ROOT_HEAD = AFS_ROOT_BUFFER;
    }

    service.memory->free(AFS_ROOT_BUFFER);

    return NO_FREE_ENTRY_FOUND;
}

uint8_t afs_update(const uint8_t* file_name, uint8_t* in, uint32_t bytes) {
    uint8_t file = find_file(file_name);
    if(file == FILE_NOT_FOUND)
        return FILE_NOT_FOUND;

    uint32_t size_in_sec = (_file.size + 511) >> 9;
    uint8_t* t_buff = service.memory->malloc(512);
    service.memory->memset(t_buff, 0, 512);

    for(uint32_t i = 0;i < size_in_sec;i++) {
        disk->write_sector(_file.start_sec + i, t_buff);
    }

    uint32_t size_in_sec_data = bytes >> 9;
    uint8_t* head = in;
    uint32_t sector = 0;
    uint32_t t_bytes = bytes;

    for(;sector < size_in_sec_data;sector++) {
        service.memory->memcpy(head, t_buff, 512);
        disk->write_sector(_file.start_sec + sector, t_buff);
        head += 512;
        t_bytes -= 512;
    }

    service.memory->memset(t_buff, 0, 512);
    service.memory->memcpy(head, t_buff, t_bytes);

    disk->write_sector(_file.start_sec + sector, t_buff);

    service.memory->free(t_buff);

    uint32_t off_sec_in_root = file_lba_index >> 5;
    uint32_t off_lba_in_sec = file_lba_index & 31;
    uint8_t* AFS_ROOT = service.memory->malloc(512);
    uint8_t* AFS_HEAD = AFS_ROOT;

    disk->read_sector(ROOT_BASE + off_sec_in_root, AFS_ROOT);

    _file.size = bytes;
    
    AFS_HEAD += (off_lba_in_sec << 4);
    service.memory->memcpy((uint8_t*)&_file, AFS_HEAD, RECORD_SIZE);
    
    disk->write_sector(ROOT_BASE + off_sec_in_root, AFS_ROOT);
    
    service.memory->free(AFS_ROOT);

    return SUCCES;
}

uint8_t afs_get_root(uint8_t *__restrict__ out) {
    uint8_t* out_head = out;
    for(uint32_t i = 0;i < ROOT_SECTORS;i++) {
        disk->read_sector(ROOT_BASE + i, out_head);
        out_head+=512;
    }
}
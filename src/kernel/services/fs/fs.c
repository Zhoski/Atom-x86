#include "fs.h"
#include "file.h"
#include "../services.h"
#include "../../device/device.h"
#include "../../string/string.h"
#include "../../../drivers/disk/pata.h"

#define FILE_TABLE_SECTOR   91
#define MAX_FILE_COUNT      16
#define FILE_NAME_LEN       16
#define FILE_TABLE_BASE     0x10000
#define TABLE_SIZE          24      // byte

file file_table[MAX_FILE_COUNT];

void init_table() {
    uint16_t data[256];

    disk_device.disk->read_sector(FILE_TABLE_SECTOR, data);
    service.memory->memcpy((uint8_t*)data, FILE_TABLE_BASE, 512); 

    uint32_t tb_offset = 0;
    uint32_t offset = 0;
    for(uint32_t tb_i = 0;tb_i < MAX_FILE_COUNT;tb_i++) {
        if((service.memory->memread(FILE_TABLE_BASE + tb_offset)) == 0) 
            break;

        for(offset = 0;offset < FILE_NAME_LEN;offset++) {
            file_table[tb_i].name[offset] = service.memory->memread(FILE_TABLE_BASE + offset + tb_offset);

        }

        file_table[tb_i].entry_sector = service.memory->memread(FILE_TABLE_BASE + offset + tb_offset);
        offset += 4;
        file_table[tb_i].size_in_sector = service.memory->memread(FILE_TABLE_BASE + offset + tb_offset);

        tb_offset += TABLE_SIZE;
    }
}

uint8_t open(const uint8_t* name) {
    for(uint32_t tb_i = 0;tb_i < MAX_FILE_COUNT;tb_i++) {
        if((strcmp(file_table[tb_i].name, name)) == 0) {
            service.vga->write_string("File found");
            uint32_t entry_sector = file_table[tb_i].entry_sector;
            return 0;
        }
    }

    service.vga->write_string("File not found");
    return 1;
}

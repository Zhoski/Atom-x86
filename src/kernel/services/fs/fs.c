#include "fs.h"
#include "file.h"
#include "../services.h"
#include "../../device/device.h"
#include "../../string/string.h"
#include "../../../drivers/disk/pata.h"
#include "../memory/program.h"

#define FILE_TABLE_SECTOR   90
#define MAX_FILE_COUNT      16
#define FILE_NAME_LEN       16
#define FILE_TABLE_BASE     0x10000
#define TABLE_SIZE          24      // byte

file file_table[MAX_FILE_COUNT];

void init_file_table() {
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
            uint32_t entry_sector = file_table[tb_i].entry_sector; // Начальный сектор
            uint32_t size_in_sector = file_table[tb_i].size_in_sector; // Размер файла в секторах
            uint32_t entry_memory = service.allocate->malloc_page(); // Тут будет программа
            uint32_t entry_stack = service.allocate->malloc_stack() + 0x2000; // Вершина стека
            uint32_t entry_offset = 0;
            uint16_t sector_buffer[256]; // Данные прочитанные из сектора

            /* Загрузка программы из диска в память */
            for(uint32_t l_sector = 0;l_sector < size_in_sector;l_sector++) {
                disk_device.disk->read_sector((entry_sector + l_sector), sector_buffer); // Читаем сектор в буффер
                service.memory->memcpy((uint8_t*)sector_buffer, (entry_memory + entry_offset), 512); // Копируем буффер в entry_memory
                entry_offset+=512;   // Сдвиг на 512 байт                                                                              
            }

            program_execute(entry_memory, entry_stack);
        }
    }

    service.vga->write_string("File not found");
    return 1;
}

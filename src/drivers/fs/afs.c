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

#define FILE_NOT_FOUND       1
#define FILE_FOUND           0

File _file;

static inline uint8_t cmpFileName(uint8_t *file_name1, uint8_t *file_name2) {
    uint8_t counter = 0;
    while(counter != 11 && *file_name1 == *file_name2) {
        counter++;
        file_name1++;
        file_name2++;
    }

    return counter == 11;
}

uint8_t find_file(const uint8_t *file_name) {
    uint8_t* AFS_ROOT = service.memory->malloc(8192);
    uint8_t* AFS_HEAD = AFS_ROOT;
    uint8_t file_find_status = FILE_NOT_FOUND;

    for(uint32_t i = 0;i < ROOT_SECTORS;i++) {
        disk.read_sector(ROOT_BASE + i, AFS_ROOT + i * 512);
    }

    while (TRUE)
    {
        if(cmpFileName(AFS_HEAD, file_name)) {
            file_find_status = FILE_FOUND;

            uint8_t* p_to_file = (uint8_t*)&_file;
            for(uint32_t i = 0;i < RECORD_SIZE;i++) {
                p_to_file[i] = AFS_HEAD[i];
            }

            break;
        }

        AFS_HEAD += RECORD_SIZE;
        if(AFS_HEAD >= (8192 + AFS_ROOT)) {
            file_find_status = FILE_NOT_FOUND;
            break;
        }
    }

    service.memory->free(AFS_ROOT);

    return file_find_status;
}

uint8_t afs_open(const uint8_t *file_name) {
    uint8_t file = find_file(file_name);
    if(file == FILE_NOT_FOUND)
        return FILE_NOT_FOUND;

    /* Загрузка файла в память */
    uint16_t buffer[256];
    uint32_t entry = 0x300000;       /* Сюда грузим программу */
    uint32_t offset = 0;
    for(uint32_t i = 0;i < _file.size_in_sec;i++) {
        disk.read_sector(_file.start_sec + i, buffer);
        service.memory->memcpy(buffer, (entry+offset), 512);
        offset += 512;
    }

    //program_spawn(entry);
    uint32_t stack = 0x400000;
    program_execute(entry, stack);
}

uint8_t read_file(const uint8_t *file_name, uint8_t *out) {
    return 0;
} 
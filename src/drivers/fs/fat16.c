#include "fat16.h"
#include "../video/video.h"
#include "../../kernel/device/device.h"
#include "../../kernel/services/services.h"
#include "../../kernel/services/memory/program.h"

#define RECORD_SIZE         32      // byte

#define TRUE                 1
#define FALSE                0

#define FILE_NOT_FOUND       1

// BPB 
uint8_t  BS_OEMName[8];  
uint16_t BPB_BytsPerSec;
uint8_t  BPB_SecPerClus;
uint16_t BPB_RsvdSecCnt;
uint8_t  BPB_NumFATs;
uint16_t BPB_RootEntCnt;
uint16_t BPB_TotSec16;
uint16_t BPB_FATSz16;
// Extended BPB
uint8_t  BS_DrvNum;
uint8_t  BS_FileSysType[8];

uint32_t RootDirStartSector;
uint32_t RootDirSectors;
uint32_t FatSectors;
uint32_t FatStartSector;
uint32_t DataStartSector;
uint32_t DataSectors;

File _file;

void init_fat16(uint16_t bootSector[256]) {
    uint8_t *bootSectorByte = (uint8_t*)bootSector;
    uint8_t byteOff = 0x03;         // Читаем начиная с 0x03

    for(;byteOff < 0x0B;byteOff++) {
        BS_OEMName[byteOff - 0x03] = bootSectorByte[byteOff];
    }
    BPB_BytsPerSec = *(uint16_t*)&bootSectorByte[0xB];
    BPB_SecPerClus = (uint8_t)bootSectorByte[0xD];
    BPB_RsvdSecCnt = *(uint16_t*)&bootSectorByte[0xE];
    BPB_NumFATs = (uint8_t)bootSectorByte[0x10];
    BPB_RootEntCnt = *(uint16_t*)&bootSectorByte[0x11];
    BPB_TotSec16 = *(uint16_t*)&bootSectorByte[0x13];
    BPB_FATSz16 = *(uint16_t*)&bootSectorByte[0x16];
    BS_DrvNum = (uint8_t)bootSectorByte[0x24];

    byteOff = 0x36;
    for(;byteOff < 0x3E;byteOff++) {
        BS_FileSysType[byteOff - 0x36] = bootSectorByte[byteOff];
    } 

    //service.vga->draw_string(BS_OEMName, 15);
    //service.vga->draw_char('\n', 15);
    //service.vga->draw_string(BS_FileSysType, 15);

    FatStartSector = BPB_RsvdSecCnt;
    FatSectors = BPB_FATSz16 * BPB_NumFATs;

    RootDirStartSector = FatStartSector + FatSectors;
    RootDirSectors = (32 * BPB_RootEntCnt + BPB_BytsPerSec - 1) / BPB_BytsPerSec; 

    DataStartSector = RootDirStartSector + RootDirSectors;
    DataSectors = BPB_TotSec16 - DataStartSector;

}

uint8_t cmpFileName(uint8_t *file_name1, uint8_t *file_name2) {
    uint8_t counter = 0;
    while(counter != 11 && *file_name1 == *file_name2) {
        counter++;
        file_name1++;
        file_name2++;
    }

    return counter == 11;
}

uint8_t load_root(uint16_t buffer[256]) {
    disk.read_sector(RootDirStartSector, buffer); 
}

uint8_t find_file(const uint8_t *file_name) {
    uint8_t buffer[512];
    uint8_t file_find_status = FALSE;
    uint8_t file_name_in_root[11];
    uint32_t i = 0;
    load_root(buffer);

    while(buffer[i] != 0) {
        for(uint32_t j = 0;j < 11;j++) {
            file_name_in_root[j] = buffer[i + j];
        }
        if(cmpFileName(file_name_in_root, file_name)) {
            file_find_status = TRUE;
            /* Копирование данных в структуру */
            for(uint32_t j = 0;j < 32;j++) {
                uint8_t *file_ptr = &_file;
                file_ptr[j] = buffer[i + j]; 
            }
            break;
        }
        i += RECORD_SIZE;
    } 

    return file_find_status;
}

uint8_t open(const uint8_t *file_name) {
    uint8_t file = find_file(file_name);
    if(!file)
        return FILE_NOT_FOUND;

    uint32_t size_in_byte = _file.size_in_b;
    uint16_t first_clus = _file.first_clus;

    uint16_t FirstSectorofCluster = DataStartSector + (first_clus - 2) * BPB_SecPerClus;
    uint16_t SizeInSec = size_in_byte / BPB_BytsPerSec;
    uint16_t SizeInSecModule = size_in_byte & BPB_BytsPerSec;
    if(SizeInSecModule != 0)
        SizeInSec++;

    //_file.ext[2] = 0;
    //_file.name[0] = 'M';
    //video.write_string(_file.name,255,255,255);

    /* Загрузка файла в память */
    uint16_t buffer[256];
    uint32_t entry = service.allocate->malloc_page();       /* Сюда грузим программу */
    uint32_t offset = 0;
    for(uint32_t i = 0;i < SizeInSec;i++) {
        disk.read_sector(FirstSectorofCluster, buffer);
        service.memory->memcpy(buffer, (entry+offset), BPB_BytsPerSec);
        offset += BPB_BytsPerSec;
    }

    //program_spawn(entry);
    uint32_t stack_top = service.allocate->malloc_stack() + 0x2000; // Верхушка стека
    program_execute(entry, stack_top);
}

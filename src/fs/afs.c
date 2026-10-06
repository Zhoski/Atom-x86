#include <fs/afs.h>
#include <drivers/disk/disk.h>
#include <drivers/video/video.h>
#include <kernel/errors.h>
#include <kernel/device.h>
#include <kernel/services.h>
#include <kernel/program.h>

#define RECORD_SIZE         16     
#define ROOT_SECTORS        16
#define ROOT_BASE            2

#define TRUE                 1
#define FALSE                0

#define FILE_DELETED      0xFF

#define NO_FREE_ENTRY_FOUND  2
#define FILE_NOT_FOUND       0
#define FILE_FOUND           1
#define SUCCES               0

File _file;
static u16 next_free_sector = 0;
static u16 file_lba_index = 0;

static inline u8 cmpFileName(u8 *__restrict__ file_name1, u8 *__restrict__ file_name2) {
    u8 counter = 0;
    while(counter != 11 && *file_name1 == *file_name2) {
        counter++;
        file_name1++;
        file_name2++;
    }

    return counter == 11;
}

u32 afs_init() {
    if(!disk) {
        return DISK_NOT_FOUND;
    }

    u8* AFS_ROOT = service.memory->malloc(8192);
    u8* AFS_ROOT_MAX = AFS_ROOT + 8192 - RECORD_SIZE;

    service.memory->memset(AFS_ROOT, 0, 8192);

    u8* AFS_HEAD = AFS_ROOT;

    io_disk_packet_t packet = {
        .command = IO_READ,
        .lba_start = ROOT_BASE,
        .sec_count = ROOT_SECTORS,
        .buffer = AFS_ROOT,
    };

    disk->dispather(&packet);

    if(packet.result) {
        return packet.result;
    }

    File* file = (File*)AFS_HEAD;
    u16 file_max_start_sec = file->start_sec;
    u16 file_size_in_sec = (file->size + 511) >> 9;

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

u32* afs_check_file(const u8 *__restrict__ file_name) {
    u8* AFS_ROOT = service.memory->malloc(8192);

    service.memory->memset(AFS_ROOT, 0, 8192);

    U8* AFS_ROOT_MAX = AFS_ROOT + 8192 - RECORD_SIZE;

    U8* AFS_HEAD = AFS_ROOT;

    io_disk_packet_t packet = {
        .command = IO_READ,
        .lba_start = ROOT_BASE,
        .sec_count = ROOT_SECTORS,
        .buffer = AFS_ROOT,
    };

    disk->dispather(&packet);

    if(packet.result) {
        return packet.result;
    }

    file_lba_index = 0;

    while (*AFS_HEAD && AFS_HEAD < AFS_ROOT_MAX)
    {
        if(*AFS_HEAD == FILE_DELETED) {
            AFS_HEAD += RECORD_SIZE;
            continue;
        }

        if(cmpFileName(AFS_HEAD, file_name)) {
            service.memory->memcpy(AFS_HEAD, &_file, RECORD_SIZE);

            service.memory->free(AFS_ROOT);
            return (U32*)&_file;
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

U8 afs_open(const U8 *file_name) {
    U8 file = afs_check_file(file_name);
    if(file == FILE_NOT_FOUND) {
        return FILE_NOT_FOUND;
    }
        

    U32 size_in_sec = (_file.size + 511) >> 9;

    /* Загрузка файла в память */
    U32 entry = 0x300000;       /* Сюда грузим программу */
    U32 offset = 0;

    io_disk_packet_t packet = {
        .command = IO_READ,
        .lba_start = _file.start_sec,
        .sec_count = size_in_sec,
        .buffer = entry
    };

    disk->dispather(&packet);

    if(packet.result) {
        return packet.result;
    }

    //program_spawn(entry);
    U32 stack = 0x500000;
    program_execute(entry, stack);
}

U8 afs_read(const U8 *__restrict__ file_name, U32 n ,U8 *__restrict__  out) {
    if(!afs_check_file(file_name)) {
        return FILE_NOT_FOUND;
    }

    U32 size_in_sec = (n + 511) >> 9;

    /*U8 *file_buffer = service.memory->malloc(512);
    U8 *head_out = out;

    U32 bytes_left = n;

    U32 i = 0;*/

    io_disk_packet_t packet = {
        .command = IO_READ,
        .lba_start = _file.start_sec,
        .sec_count = size_in_sec,
        .buffer = out,
    };

    disk->dispather(&packet);
    
    if(packet.result) {
        return packet.result;
    }

    /*for(;i < size_in_sec;i++) {
        disk->read_sector(_file.start_sec + i, file_buffer);

        U32 chunk = (bytes_left > 512) ? 512 : bytes_left;

        service.memory->memcpy(file_buffer, head_out, chunk);
        head_out += chunk;
        bytes_left -= chunk;
    }

    service.memory->free(file_buffer);*/

    return SUCCES;
} 

U8 afs_delete(const U8 *__restrict__ file_name) {
    U8* AFS_ROOT_BUFFER = service.memory->malloc(512);
    U8* AFS_ROOT_HEAD = AFS_ROOT_BUFFER;

    io_disk_packet_t packet = {
        .command = IO_READ,
        .buffer = AFS_ROOT_BUFFER,
        .sec_count = 1,
    };

    for(U32 sector = ROOT_BASE; sector < ROOT_BASE + ROOT_SECTORS;sector++) { 
        //disk->read_sector(sector, AFS_ROOT_BUFFER);
        packet.lba_start = sector;
        disk->dispather(&packet);

        for(U32 i = 0;i < 32;i++) {
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

U8 afs_create(const U8 *__restrict__ file_name, uint16_t size) {
    U8* AFS_ROOT_BUFFER = service.memory->malloc(512);
    U8* AFS_ROOT_HEAD = AFS_ROOT_BUFFER;

    io_disk_packet_t packet = {
        .command = IO_READ,
        .buffer = AFS_ROOT_BUFFER,
        .sec_count = 1,
    };

    video->kprintf("Creating\n");

    for(U32 sector = ROOT_BASE; sector < ROOT_BASE + ROOT_SECTORS;sector++) {   
        //disk->read_sector(sector, AFS_ROOT_BUFFER);
        packet.lba_start = sector;
        disk->dispather(&packet);

        for(U32 i = 0; i < 32;i++) {
            if(!*AFS_ROOT_HEAD || *AFS_ROOT_HEAD == FILE_DELETED) {
                File new_file;

                service.memory->memcpy(file_name, (U8*)&new_file, 11);
                new_file.size = size;
                new_file.start_sec = next_free_sector;
                new_file.flags = 0x1;

                next_free_sector += (size + 511) >> 9;

                service.memory->memcpy((U8*)&new_file, AFS_ROOT_HEAD, 16);

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

U8 afs_update(const U8 *__restrict__ file_name, U8 *__restrict__ in, U32 bytes) {
    U8 file = afs_check_file(file_name);
    if(file == FILE_NOT_FOUND)
        return FILE_NOT_FOUND;

    U32 size_in_sec = (_file.size + 511) >> 9;
    u8* buff = service.memory->malloc(bytes);
    service.memory->memset(buff, 0, bytes);

    io_disk_packet_t packet = {
        .command = IO_WRITE,
        .lba_start = _file.start_sec,
        .sec_count = size_in_sec,
        .buffer = buff,
    };

    disk->dispather(&packet);

    U32 size_in_sec_data = bytes >> 9;
    U8* head = in;
    U32 sector = 0;
    U32 bytes_left = bytes;

    packet.buffer = in;


    U32 off_sec_in_root = file_lba_index >> 5;
    U32 off_lba_in_sec = file_lba_index & 31;
    U8* AFS_ROOT = service.memory->malloc(512);
    U8* AFS_HEAD = AFS_ROOT;

    disk->read_sector(ROOT_BASE + off_sec_in_root, AFS_ROOT);

    _file.size = bytes;
    
    AFS_HEAD += (off_lba_in_sec << 4);
    service.memory->memcpy((U8*)&_file, AFS_HEAD, RECORD_SIZE);

    packet.lba_start = ROOT_BASE + off_sec_in_root;
    packet.buffer = AFS_ROOT;

    disk->dispather(&packet);
    
    service.memory->free(AFS_ROOT);

    return SUCCES;
}

U8 afs_get_root(U8 *__restrict__ out) {
    io_disk_packet_t packet = {
        .command = IO_READ,
        .lba_start = ROOT_BASE,
        .sec_count = ROOT_SECTORS,
        .buffer = out,
    };

    disk->dispather(&packet);
}

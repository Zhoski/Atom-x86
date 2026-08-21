#include <drivers/disk/disk.h>
#include <drivers/disk/ata.h>

#define DISK_NOT_FOUND      1
#define SUCCES_INIT_DISK    0

static Disk ATA_PIO = {
    .init = &init_ata,
    .read_sector = &ata_read_sector,
    .write_sector = &ata_write_sector
};

Disk* disk;

uint8_t disk_init(uint16_t disk_info[256]) {
    disk = &ATA_PIO;
    if(!disk->init) {
        return DISK_NOT_FOUND;
    }else {
        disk->init(disk_info);
        return SUCCES_INIT_DISK;
    }
}

void disk_handler() {
    return;
}
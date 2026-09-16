#include <drivers/disk/disk.h>
#include <drivers/disk/sata.h>
#include <drivers/disk/ata.h>
#include <drivers/pci/pci.h>
#include <drivers/video/video.h>

static Disk ATA_PIO = {
    .init = &init_ata,
    .read_sector = &ata_read_sector,
    .write_sector = &ata_write_sector
};

static Disk SATA_AHCI = {
    .init = &init_sata,
    .read_sector = &sata_read_sector,
    .write_sector = &sata_write_sector
};

Disk* disk;

uint8_t disk_init(uint16_t disk_info[256]) {
    disk = 0;
    U32 sata_detect = 0;
    U32 ata_detect = 0;
    for(U32 device = 0; device < 128; device++) {
        if(pci_devices[device].class == 0x01 && pci_devices[device].subclass == 0x01) {
            if((pci_devices[device].bar0 == 0 || pci_devices[device].bar0 <= 1) & !ata_detect) {
                disk_bar0 = 0x1F0;
            }else {
                disk_bar0 = pci_devices[device].bar0;
            }
            ata_detect = 1;
        }
        else if(pci_devices[device].class == 0x01 && pci_devices[device].subclass == 0x06) {
            sata_detect = 1;
            ahci_mem_base = pci_devices[device].bar5;
            break;
        }
    }

    if(sata_detect) {
        disk = &SATA_AHCI;
    }else {
        disk = &ATA_PIO;
    }
    
    if(!ata_detect && !sata_detect) {
        return DISK_NOT_FOUND;
    }else {
        return disk->init(disk_info);
    }
}

void disk_handler() {
    return;
}
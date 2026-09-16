#include <drivers/disk/disk.h>
#include <drivers/disk/ata.h>
#include <drivers/pci/pci.h>

static Disk ATA_PIO = {
    .init = &init_ata,
    .read_sector = &ata_read_sector,
    .write_sector = &ata_write_sector
};

Disk* disk;

uint8_t disk_init(uint16_t disk_info[256]) {
    disk = 0;
    for(U32 device = 0; device < 128; device++) {
        if(pci_devices[device].class == 0x01 && pci_devices[device].subclass == 0x01) {
            disk = &ATA_PIO;
            if(pci_devices[device].bar0 == 0 || pci_devices[device].bar0 <= 1) {
                disk_bar0 = 0x1F0;
            }else {
                disk_bar0 = pci_devices[device].bar0;
            }
            break;
        }
        else if(pci_devices[device].class == 0x01 && pci_devices[device].subclass == 0x06) {
            return DISK_DONT_SUPPORT_PATA;
        }
    }

    if(!disk->init) {
        return DISK_NOT_FOUND;
    }else {
        return disk->init(disk_info);
    }
}

void disk_handler() {
    return;
}
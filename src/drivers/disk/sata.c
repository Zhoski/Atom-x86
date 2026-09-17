#include <drivers/disk/sata.h>
#include <drivers/video/video.h>
#include <drivers/timer/timer.h>

#define SATA_SIG_ATA    0x00000101
#define SATA_SIG_ATAPI  0xEB140101
#define SATA_SIG_PM     0x96A50101
#define SATA_SIG_SEMB   0xC33C0101


struct HBA_mem* hba_mem;

// Сброс порта
void ahci_port_reset(U32 port) {

    // cmd.st сбросить
    hba_mem->port[port].cmd &= ~0x01;

    // Ожидание сброса cmd.cr
    while (1)
    {
        if(hba_mem->port->cmd & 0x8000) continue;
        break;
    }

    // сигнал COMRESET
    hba_mem->port[port].sctl &= ~0x0F;
    hba_mem->port[port].sctl |= 0x01;
    ksleep(5);

    hba_mem->port[port].sctl &= ~0x0F;

    // Ждем соединения
    for(U32 timeout = 100; timeout > 0; timeout--) {
        ksleep(1);
        if((hba_mem->port[port].ssts & 0xF) == 0x03) {
            break;
        }
    }
    
    hba_mem->port[port].serr = 0xFFFFFFFF;
}

void ahci_scan_port() {
    for(U32 i = 0; i < 32; i++) {
        if(hba_mem->pi & (1 << i)) {
            ahci_port_reset(i);
            video->write_string("Device found port: ");
            video->write_int(i);
            video->write_string("   Type: ");
            if(hba_mem->port[i].sig == SATA_SIG_ATA) {
                video->write_string("ATA DEVICE\n");
            }
            else if(hba_mem->port[i].sig == SATA_SIG_ATAPI) {
                video->write_string("ATAPI DEVICE\n");
            }
            else if(hba_mem->port[i].sig == SATA_SIG_PM) {
                video->write_string("PM DEVICE\n");
            }
            else if(hba_mem->port[i].sig == SATA_SIG_SEMB) {
                video->write_string("SEMB DEVICE\n");
            }else {
                video->write_string("NO DEVICE\n");
            }
        }
    }
    
}

U32 init_sata(U16 info[256]) {
    hba_mem = (U32*)ahci_mem_base;

    hba_mem->ghc |= 0x80000000;

    ahci_scan_port();

    return 2;
}
U8 sata_read_sector(U32 lba, U16 word[256]);
U8 sata_write_sector(U32 lba, U16 word[256]);
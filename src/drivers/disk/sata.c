#include <drivers/disk/sata.h>
#include <drivers/video/video.h>
#include <drivers/timer/timer.h>
#include <kernel/memory.h>

#define CLB_MEM_BASE       0x10000
#define FIS_MEM_BASE       0x20000
#define CMD_MEM_BASE       0x30000

#define SATA_SIG_ATA    0x00000101
#define SATA_SIG_ATAPI  0xEB140101
#define SATA_SIG_PM     0x96A50101
#define SATA_SIG_SEMB   0xC33C0101

#define H2D                   0x27

#define IDENTIFY_DEVICE       0xEC

struct HBA_mem* hba_mem;
struct HBA_cmd_header* cmd_header;
struct HBA_cmd_table* cmd_table;
struct HBA_fis_layout* fis_layout;

U8 buffer[512] __attribute__((aligned(4)));

// Сброс порта
void ahci_port_reset(U32 port) {
    // cmd.st сбросить
    hba_mem->port[port].cmd &= ~0x01;

    // Ожидание сброса cmd.cr
    while (1)
    {
        if(hba_mem->port[port].cmd & 0x8000) continue;
        break;
    }

    // сигнал COMRESET
    hba_mem->port[port].sctl &= ~0x0F;
    hba_mem->port[port].sctl |= 0x01;
    ksleep(5);

    hba_mem->port[port].sctl &= ~0x0F;

    // Ждем соединения
    for(U32 timeout = 10; timeout > 0; timeout--) {
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
                memset(CLB_MEM_BASE, 0, 1024);
                memset(FIS_MEM_BASE, 0, 256);

                hba_mem->port[i].cmd &= ~0x01;

                while (1)
                {
                    if(hba_mem->port[i].cmd & 0x8000) continue;
                    break;
                }

                hba_mem->port[i].cmd &= ~0x10;

                // Ожидание сброса cmd.cr cmd.fr
                while (1)
                {
                    if(hba_mem->port[i].cmd & 0x4000) continue;
                    break;
                }

                hba_mem->port[i].clb = CLB_MEM_BASE;
                hba_mem->port[i].clbu = 0;

                hba_mem->port[i].fb = FIS_MEM_BASE;
                hba_mem->port[i].fbu = 0;

                hba_mem->port[i].cmd |= 0x10; 

                while (hba_mem->port[i].tfd & (0x80 | 0x08)); 

                hba_mem->port[i].cmd |= 0x01;
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

void anci_identify_device(U32 ncs) {
    U32 free_slot = 0;
    U32 cur_slot = 0;
    U32 cur_slot_detect = 0;

    for(U32 slot = 0; slot < ncs; slot++) {
        if(!(hba_mem->port[0].ci & (1 << slot))) {
            if(!cur_slot_detect) {
                cur_slot_detect = 1;
                cur_slot = slot;
            }

            free_slot++;
        }
    }

    cmd_header->prdtl = 1;
    
    // Размер команды 5 двойных слов
    cmd_header->w0 &= ~0x1F;
    cmd_header->w0 |= 0x05;

    cmd_header->w0 &= ~0x20;    // бит A сбросить

    cmd_header->w0 &= ~0x40;    // бит W сбросить для чтения

    cmd_table->prdt_entry.dba = 0x40000;    // Сюда придут данные о диске
    cmd_table->prdt_entry.dbau = 0;

    cmd_table->prdt_entry.dbc = 511;        // Читаем 512 байт
    cmd_table->prdt_entry.i = 0;            // Прерывания отключить

    cmd_table->cfis[0] = H2D;   // От хоста к диску
    cmd_table->cfis[1] = 0x80;  
    cmd_table->cfis[2] = IDENTIFY_DEVICE;
    cmd_table->cfis[7] = 0xA0;

    cmd_header->prdbc = 0;

    while (hba_mem->port[0].tfd & (0x80 | 0x08));

    hba_mem->port[0].ci = 1;

    while (hba_mem->port[0].ci & 0x01);
    
    U16* identify_buffer = 0x40000;

    video->write_string("Disk: ");

    for(U32 i = 27; i < 46; i++) {
        U8 low = (identify_buffer[i] >> 8) & 0xFF;
        U8 high = identify_buffer[i] & 0xFF;

        video->write_char(low);
        video->write_char(high);
    }

    video->write_char('\n');
}

U32 init_sata(U16 info[256]) {
    hba_mem = (U32*)ahci_mem_base;
    cmd_header = (U32*)CLB_MEM_BASE;
    fis_layout = (U32*)FIS_MEM_BASE;
    cmd_table = (U32*)CMD_MEM_BASE;

    memset(CMD_MEM_BASE, 0, 128);

    hba_mem->ghc |= 0x80000000;

    U32 ncs = ((hba_mem->cap >> 8) & 0x1F) + 1;

    video->write_string("Number of command slot: ");
    video->write_int(ncs);
    video->write_char('\n');

    ahci_scan_port();

    /*video->write_string("\nNumber of free slot: ");
    video->write_int(free_slot);
    video->write_char('\n');

    video->write_string("Current slot: ");
    video->write_int(cur_slot);
    video->write_char('\n');*/

    cmd_header->ctba = CMD_MEM_BASE;
    cmd_header->ctbau = 0;

    anci_identify_device(ncs);

    return 2;
}
U8 sata_read_sector(U32 lba, U16 word[256]);
U8 sata_write_sector(U32 lba, U16 word[256]);
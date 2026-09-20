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

void ahci_ok_log(u8* msg) {
    video->write_string("[  ");
    video->terminal_fg_vbe_set(10);
    video->write_string("OK");
    video->terminal_fg_vbe_set(15);
    video->write_string("  ] ");

    video->write_string("AHCI Init: ");
    video->write_string(msg);
}

void ahci_info_log(u8* msg) {
    video->write_string("[ ");
    video->write_string("INFO");
    video->write_string(" ] ");

    video->write_string("AHCI Init: ");
    video->write_string(msg);
}

void ahci_fail_log(u8* msg) {
    video->write_string("[ ");
    video->terminal_fg_vbe_set(12);
    video->write_string("FAIL");
    video->terminal_fg_vbe_set(15);
    video->write_string(" ] ");

    video->write_string("AHCI Init: ");
    video->write_string(msg);
}

/**
 *  Проверяет статус устройства на порту считывая регистр ssts
 */
U8* ahci_port_get_status(U32 port) {
    U32 status = hba_mem->port[port].ssts & 0x0F;

    switch (status)
    {
        case 0x00: return "No device detected";
        case 0x01: return "Device detected, but Phy communication not established";
        case 0x03: return "Device found and link established (Ready)";
        default:   return "Unknown or transition state";
    }
}

/**
 *  Выключает DMA-движок комманд порта (ST = 0)
 *  И ожидает сброса регистра CR
 */
void ahci_port_dma_disable(u32 port) {
    hba_mem->port[port].cmd &= ~0x10;

    while (hba_mem->port[0].cmd & 0x4000);

    hba_mem->port[port].cmd &= ~0x01;

    while (hba_mem->port[0].cmd & 0x8000);
}

/**
 *  Запускает DMA-движок комманд порта (ST = 1)
 *  И ожидает взвода регистра CR
 */
void ahci_port_dma_enable(u32 port) {
    hba_mem->port[port].cmd |= 0x10;

    while (!(hba_mem->port[0].cmd & 0x4000));

    hba_mem->port[port].cmd |= 0x01;

    while (!(hba_mem->port[0].cmd & 0x8000));
}

/**
 *   Выполняет аппаратный сброс SATA-порта (COMRESET)
 *   Останавливает DMA-движок порта и вызывает физический сброс линии 
 */
void ahci_port_reset(u32 port) {
    hba_mem->port[port].cmd &= ~0x01;

    while (1)
    {
        if(hba_mem->port[port].cmd & 0x8000) continue;
        break;
    }

    hba_mem->port[port].sctl &= ~0x0F;
    hba_mem->port[port].sctl |= 0x01;
    ksleep(5);

    hba_mem->port[port].sctl &= ~0x0F;

    for(U32 timeout = 10; timeout > 0; timeout--) {
        ksleep(1);
        if((hba_mem->port[port].ssts & 0xF) == 0x03) {
            break;
        }
    }
    
    hba_mem->port[port].serr = 0xFFFFFFFF;
}

/** 
 * Сканирует все существующие порты, определяет тип устройства на каждом порту
 * Сбрасывает порт и приводит его в состояние готовности
*/
void ahci_port_scan() {
    ahci_info_log("Scan port...\n");
    for(u32 i = 0; i < 32; i++) {
        if(hba_mem->pi & (1 << i)) {
            ahci_port_reset(i);
            ahci_info_log("Device found port: ");
            video->write_int(i);
            video->write_string("   Type: ");
            if(hba_mem->port[i].sig == SATA_SIG_ATA) {
                video->write_string("SATA DEVICE\n");
                memset(CLB_MEM_BASE, 0, 1024);
                memset(FIS_MEM_BASE, 0, 256);

                hba_mem->port[i].cmd &= ~0x01;

                while (1)
                {
                    if(hba_mem->port[i].cmd & 0x8000) continue;
                    break;
                }

                hba_mem->port[i].cmd &= ~0x10;

                while (1)
                {
                    if(hba_mem->port[i].cmd & 0x4000) continue;
                    break;
                }

                hba_mem->port[i].clb = CLB_MEM_BASE;
                hba_mem->port[i].clbu = 0;

                hba_mem->port[i].fb = FIS_MEM_BASE;
                hba_mem->port[i].fbu = 0;

                cmd_header->ctba = CMD_MEM_BASE & ~0x7F;
                cmd_header->ctbau = 0;

                hba_mem->port[0].cmd |= 0x10; 

                while (hba_mem->port[0].tfd & (0x80 | 0x08)); 

                hba_mem->port[0].cmd |= 0x01;
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

/**
 * Составляет и отправляет команду 0xEC для идентификации устройства
 * Паспорт устройства приходит на 0x400000
 */
void anci_identify_device(u32 ncs) {
    u8* port_status = ahci_port_get_status(0);

    ahci_info_log(port_status);

    video->write_char('\n');

    u32 free_slot = 0;
    u32 cur_slot = 0;
    u32 cur_slot_detect = 0;

    for(u32 slot = 0; slot < ncs; slot++) {
        if (!(hba_mem->port[0].ci & (1 << slot)) && !(hba_mem->port[0].sact & (1 << slot))) {
            if(!cur_slot_detect) {
                cur_slot_detect = 1;
                cur_slot = slot;
            }

            free_slot++;
        }
    }

    ahci_info_log("Free slot: ");
    video->write_int(free_slot);
    video->write_char('\n');
    ahci_info_log("Selected slot: ");
    video->write_int(cur_slot);
    video->write_char('\n');

    cmd_header->prdtl = 1;
    
    cmd_header->w0 = 5;

    cmd_table->prdt_entry.dba = 0x400000;   
    cmd_table->prdt_entry.dbau = 0;

    cmd_table->prdt_entry.dbc = 511;    
    cmd_table->prdt_entry.i = 0;         

    cmd_table->cfis[0] = H2D;
    cmd_table->cfis[1] = 0x80;  
    cmd_table->cfis[2] = IDENTIFY_DEVICE;
    cmd_table->cfis[7] = 0xA0;

    cmd_header->prdbc = 0;

    hba_mem->port[0].is = 0xFFFFFFFF; 

    while (hba_mem->port[0].tfd & (0x80 | 0x08)); 

    ahci_port_dma_enable(0);

    hba_mem->port[0].ci = 1;

    u32 step = 0;

    while (hba_mem->port[0].ci & 0x01) {
        if(step == 10) {
            ahci_fail_log("Disk timeout\n");
            for(;;) {
                asm("hlt");
            }
        }

        if(hba_mem->port[0].tfd & 0x01) {
            ahci_fail_log("Disk tfd error: ");
            video->write_int((hba_mem->port[0].tfd >> 8) & ~0xFFFF00);
            video->write_char('\n');
            break;
        }

        if(hba_mem->port[0].serr) {
            ahci_fail_log("Disk serr error: ");
            video->write_int(hba_mem->port[0].serr);
            video->write_char('\n');
            break;
        }

        step++;
        ksleep(5);
    }
    
    u16* identify_buffer = 0x400000;

    ahci_info_log("Device: ");

    for(u32 i = 27; i < 46; i++) {
        u8 low = (identify_buffer[i] >> 8) & 0xFF;
        u8 high = identify_buffer[i] & 0xFF;

        video->write_char(low);
        video->write_char(high);
    }

    video->write_char('\n');
}

/**
 *  Инициализирует SATA-контроллер и получает его паспорт
 *  Заполняет буффер info паспортом устройства полученным через IDENTIFY
 */
u32 init_sata(u16 info[256]) {
    video->write_string("AHCI Driver v0.0.8\n");

    hba_mem = (struct HBA_mem*)ahci_mem_base;
    cmd_header = (struct HBA_cmd_header*)CLB_MEM_BASE;
    fis_layout = (struct HBA_fis_layout*)FIS_MEM_BASE;
    cmd_table = (struct HBA_cmd_table*)CMD_MEM_BASE;

    memset(CMD_MEM_BASE, 0, 256);

    hba_mem->ghc |= 0x80000000;

    u32 ncs = ((hba_mem->cap >> 8) & 0x1F) + 1;

    ahci_port_scan();

    anci_identify_device(ncs);

    return 2;
}
u8 sata_read_sector(u32 lba, u16 word[256]);
u8 sata_write_sector(u32 lba, u16 word[256]);
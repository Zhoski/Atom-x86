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
#define READ                  0x25
#define WRITE                 0xC5

#define AHCI_DRIVER_V_MAJOR      0
#define AHCI_DRIVER_V_MINOR      1        
#define AHCI_DRIVER_V_PATCH      0        

struct HBA_mem* hba_mem;
struct HBA_cmd_header* cmd_header;
struct HBA_cmd_table* cmd_table;
struct HBA_fis_layout* fis_layout;
struct FIS_H2D* fis_h2d;

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

    for(u32 timeout = 10; timeout > 0; timeout--) {
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
    video->kprintf("[ INFO ] AHCI: Scan bus...\n");
    for(u32 port = 0; port < 32; port++) {
        if(hba_mem->pi & (1 << port)) {
            ahci_port_reset(port);
            video->kprintf("[ INFO ] AHCI: Device found port: %d    Type: ", port);
            if(hba_mem->port[port].sig == SATA_SIG_ATA) {
                video->kprintf("SATA DEVICE\n");
                memset(CLB_MEM_BASE, 0, 1024);
                memset(FIS_MEM_BASE, 0, 256);

                hba_mem->port[port].cmd &= ~0x01;

                while (1)
                {
                    if(hba_mem->port[port].cmd & 0x8000) continue;
                    break;
                }

                hba_mem->port[port].cmd &= ~0x10;

                while (1)
                {
                    if(hba_mem->port[port].cmd & 0x4000) continue;
                    break;
                }

                hba_mem->port[port].clb = CLB_MEM_BASE;
                hba_mem->port[port].clbu = 0;

                hba_mem->port[port].fb = FIS_MEM_BASE;
                hba_mem->port[port].fbu = 0;

                cmd_header->ctba = CMD_MEM_BASE & ~0x7F;
                cmd_header->ctbau = 0;

                hba_mem->port[0].cmd |= 0x10; 

                while (hba_mem->port[0].tfd & (0x80 | 0x08)); 

                hba_mem->port[0].cmd |= 0x01;
            }
            else if(hba_mem->port[port].sig == SATA_SIG_ATAPI) {
                video->kprintf("ATAPI DEVICE\n");
            }
            else if(hba_mem->port[port].sig == SATA_SIG_PM) {
                video->kprintf("PM DEVICE\n");
            }
            else if(hba_mem->port[port].sig == SATA_SIG_SEMB) {
                video->kprintf("SEMB DEVICE\n");
            }else {
                video->kprintf("NO DEVICE\n");
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

    video->kprintf("[ INFO ] AHCI: %s\n", port_status);

    u32 free_slot = 0;
    u32 selected_slot = 0;
    u32 selected_slot_detect = 0;

    for(u32 slot = 0; slot < ncs; slot++) {
        if (!(hba_mem->port[0].ci & (1 << slot)) && !(hba_mem->port[0].sact & (1 << slot))) {
            if(!selected_slot_detect) {
                selected_slot_detect = 1;
                selected_slot = slot;
            }

            free_slot++;
        }
    }

    video->kprintf("[ INFO ] AHCI: Free command slot: %d\n", free_slot);
    video->kprintf("[ INFO ] AHCI: Selected slot: %d\n", selected_slot);

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
            video->kprintf("[ %f12FAIL%f15 ] AHCI: Disk timeout\n");
            for(;;) {
                asm("hlt");
            }
        }

        if(hba_mem->port[0].tfd & 0x01) {
            video->kprintf("[ %f12FAIL%f15 ] AHCI: Disk tfd error: %d\n", (hba_mem->port[0].tfd >> 8) & ~0xFFFF00);
            break;
        }

        if(hba_mem->port[0].serr) {
            video->kprintf("[ %f12FAIL%f15 ] AHCI: Disk serr error: %d\n", hba_mem->port[0].serr);
            break;
        }

        step++;
        ksleep(5);
    }
    
    u16* identify_buffer = 0x400000;

    video->kprintf("[ INFO ] AHCI: Device: ");

    for(u32 i = 27; i < 46; i++) {
        u8 low = (identify_buffer[i] >> 8) & 0xFF;
        u8 high = identify_buffer[i] & 0xFF;

        video->write_char(low);
        video->write_char(high);
    }

    video->kprintf("\n");
}

/**
 *  Инициализирует SATA-контроллер и получает его паспорт
 *  Заполняет буффер info паспортом устройства полученным через IDENTIFY
 */
u32 init_sata(u16 info[256]) {
    video->kprintf("AHCI driver v %d.%d.%d\n", AHCI_DRIVER_V_MAJOR, AHCI_DRIVER_V_MINOR, AHCI_DRIVER_V_PATCH);

    hba_mem = (struct HBA_mem*)ahci_mem_base;
    cmd_header = (struct HBA_cmd_header*)CLB_MEM_BASE;
    fis_layout = (struct HBA_fis_layout*)FIS_MEM_BASE;
    cmd_table = (struct HBA_cmd_table*)CMD_MEM_BASE;
    fis_h2d = &cmd_table->cfis;

    memset(CMD_MEM_BASE, 0, 256);

    hba_mem->ghc |= 0x80000000;

    u32 ncs = ((hba_mem->cap >> 8) & 0x1F) + 1;

    ahci_port_scan();

    anci_identify_device(ncs);

    return 0;
}
u8 ahci_sector_read(u64 lba, u16 word[256]) { 
    cmd_header->w0 = 0x5;

    fis_h2d->type =  H2D;
    fis_h2d->flag = 0x80;
    fis_h2d->cmd =  READ;

    fis_h2d->lba0 = lba & 0xFF;
    fis_h2d->lba1 = (lba >> 8) & 0xFF;
    fis_h2d->lba2 = (lba >> 16) & 0xFF;
    fis_h2d->lba3 = (lba >> 24) & 0xFF;
    fis_h2d->lba4 = (lba >> 32) & 0xFF;
    fis_h2d->lba5 = (lba >> 40) & 0xFF;

    fis_h2d->dev = 0x40;

    fis_h2d->sec_count_low = 1;
    fis_h2d->sec_count_hight = 0;

    hba_mem->port[0].ci = 1;

    u32 step = 0;

    while (hba_mem->port[0].ci & 0x01) {
        if(step == 10) {
            video->kprintf("[ %f12FAIL%f15 ] AHCI: Disk timeout\n");
            for(;;) {
                asm("hlt");
            }
        }

        if(hba_mem->port[0].tfd & 0x01) {
            video->kprintf("[ %f12FAIL%f15 ] AHCI: Disk tfd error: %d\n", (hba_mem->port[0].tfd >> 8) & ~0xFFFF00);
            break;
        }

        if(hba_mem->port[0].serr) {
            video->kprintf("[ %f12FAIL%f15 ] AHCI: Disk serr error: %d\n", hba_mem->port[0].serr);
            break;
        }

        step++;
        ksleep(1);
    }

    memcpy(0x400000, word, 512);

    return 0;
    
}
u8 ahci_sector_write(u64 lba, u16 word[256]) {
    memcpy(word, 0x400000, 512);

    cmd_header->w0 = 0x45;

    fis_h2d->type =  H2D;
    fis_h2d->flag = 0x80;
    fis_h2d->cmd =  READ;

    fis_h2d->lba0 = lba & 0xFF;
    fis_h2d->lba1 = (lba >> 8) & 0xFF;
    fis_h2d->lba2 = (lba >> 16) & 0xFF;
    fis_h2d->lba3 = (lba >> 24) & 0xFF;
    fis_h2d->lba4 = (lba >> 32) & 0xFF;
    fis_h2d->lba5 = (lba >> 40) & 0xFF;

    fis_h2d->dev = 0x40;

    fis_h2d->sec_count_low = 1;
    fis_h2d->sec_count_hight = 0;

    hba_mem->port[0].ci = 1;

    u32 step = 0;

    while (hba_mem->port[0].ci & 0x01) {
        if(step == 10) {
            video->kprintf("[ %f12FAIL%f15 ] AHCI: Disk timeout\n");
            for(;;) {
                asm("hlt");
            }
        }

        if(hba_mem->port[0].tfd & 0x01) {
            video->kprintf("[ %f12FAIL%f15 ] AHCI: Disk tfd error: %d\n", (hba_mem->port[0].tfd >> 8) & ~0xFFFF00);
            break;
        }

        if(hba_mem->port[0].serr) {
            video->kprintf("[ %f12FAIL%f15 ] AHCI: Disk serr error: %d\n", hba_mem->port[0].serr);
            break;
        }

        step++;
        ksleep(1);
    }

    return 0;
}
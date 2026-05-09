#include "pata.h"
#include "../../kernel/port/io.h"
#include "../../kernel/device/device.h"
#include "../../kernel/services/services.h"

#define BSY                     0x80
#define ERR                      0x1
#define DRQ                      0x8
#define DRIVE                   0xA0
#define IDENTIFY                0xEC

#define ATA_PRIMARY_DATA       0x1F0
#define ATA_PRIMARY_STATUS     0x1F7

#define BUFFER_SIZE              256

#define SUCCESS                    0
#define DISK_NOT_FOUND             1
#define DISK_DONT_SUPPORT_PATA     2
#define DISK_ERROR                 3

static disk_interface disk_ops = {
    .init_pata = &init_pata,
    .read_sector = &read_sector,
    .write_sector = &write_sector,
};

device disk_device;

uint8_t init_pata(uint16_t info[256]) {
    /* IDENTIFY */
    outb(0x1F6, DRIVE);
    outb(0x1F2, 0);
    outb(0x1F3, 0); 
    outb(0x1F4, 0);
    outb(0x1F5, 0);
    
    /* Задержка 400 нс */
    for(uint32_t i = 0;i < 4;i++) inb(ATA_PRIMARY_STATUS); 

    outb(ATA_PRIMARY_STATUS, IDENTIFY);

    uint8_t exit_status = SUCCESS;
   
    /* Если 0x1F7 установлен на ноль, то диска не существует */
    if(inb(ATA_PRIMARY_STATUS) == 0) {
        exit_status = DISK_NOT_FOUND;
        goto exit; 
    }

    /* Ждем пока BSY установится на ноль */
    while((inb(ATA_PRIMARY_STATUS) & BSY));

    /* Если 0x1F4 и 0x1F5 равны нулю, то диск не поддерживает PATA */
    if(inb(0x1F4) != 0 && inb(0x1F5) != 0) {
        exit_status = DISK_DONT_SUPPORT_PATA;
        goto exit; 
    }

    uint8_t status;

    /* Ждеми 1 в DRQ если успешно, или 1 в ERR в случаи ошибки */
    while(1) {
        status = inb(ATA_PRIMARY_STATUS);
        if(status & DRQ) break;
        if(status & ERR) {
            exit_status = DISK_ERROR;
            goto exit;
        }
    }

    /* Регистрация устройства */
    disk_device.name = "HDD0";
    disk_device.disk = &disk_ops;

    //uint16_t info[BUFFER_SIZE];
    
    /* Читаем данные о диске из 0x1F0 в буффер */
    for(uint32_t i = 0;i < BUFFER_SIZE;i++) {
        info[i] = inw(ATA_PRIMARY_DATA);
    } 

exit:
    return exit_status;
}
uint8_t read_sector(uint32_t lba, uint16_t word[256]) {
    // Установить устройство
    uint8_t master_slave = 0b1110;
    uint8_t drive_head = 0xE0 | (master_slave << 4) | ((lba >> 24) & 0x0F);

    outb(0x1F6, drive_head);

    outb(0x1F2, 1);                  // Читать 1 сектор
    outb(0x1F3, (uint8_t)lba);       // Младшая часть lba
    outb(0x1F4, (uint8_t)lba >> 8);  // Средняя часть lba
    outb(0x1F5, (uint8_t)lba >> 16); // Старшая часть lba 
    outb(ATA_PRIMARY_STATUS, 0x20);  // Читать
    
    while((inb(ATA_PRIMARY_STATUS) & BSY));
    uint8_t status;
    uint8_t exit_status;
    while(1) {
        status = inb(ATA_PRIMARY_STATUS);
        if(status & DRQ) break;
        if(status & ERR) {
            exit_status = DISK_ERROR;
            return exit_status;
        }
    }

    for(uint32_t i = 0;i < 256;i++) {
        word[i] = inw(0x1F0);
    }
}
void write_sector(uint32_t lba, uint16_t word[256]) {
    outb(0x1F2, 1);                  // Читать 1 сектор
    outb(0x1F3, (uint8_t)lba);       // Младшая часть lba
    outb(0x1F4, (uint8_t)lba >> 8);  // Средняя часть lba
    outb(0x1F5, (uint8_t)lba >> 16); // Старшая часть lba 
    outb(0x1F7, 0x30);               // Читать

    //while((inb(0x1F7) & BSY));
    while(!(inb(0x1F7) & 0x8));

    for(uint32_t i = 0;i < 256;i++) {
        outw(0x1F0, word[i]);
    }
}

void disk_handler() {
    return;
}

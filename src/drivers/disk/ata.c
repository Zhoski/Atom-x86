#include <drivers/disk/ata.h>
#include <cpu/io.h>

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

#define READ                    0x20
#define WRITE                   0x30

U8 init_ata(U16 info[256]) {
    /* IDENTIFY */
    outb(0x1F6, DRIVE);
    outb(0x1F2, 0);
    outb(0x1F3, 0); 
    outb(0x1F4, 0);
    outb(0x1F5, 0);
    
    /* Задержка 400 нс */
    inb(ATA_PRIMARY_STATUS);
    inb(ATA_PRIMARY_STATUS);
    inb(ATA_PRIMARY_STATUS);
    inb(ATA_PRIMARY_STATUS);

    outb(ATA_PRIMARY_STATUS, IDENTIFY);

    U8 exit_status = SUCCESS;
   
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

    U8 status;

    /* Ждеми 1 в DRQ если успешно, или 1 в ERR в случаи ошибки */
    while(1) {
        status = inb(ATA_PRIMARY_STATUS);
        if(status & DRQ) break;
        if(status & ERR) {
            exit_status = DISK_ERROR;
            goto exit;
        }
    }
    
    /* Читаем данные о диске из 0x1F0 в буффер */
    for(U32 i = 0; i < BUFFER_SIZE; i++) {
        info[i] = inw(ATA_PRIMARY_DATA);
    } 

exit:
    return exit_status;
}

U8 ata_read_sector(U32 lba, U16 word[256]) {
    // Установить устройство
    U8 drive_head = 0xE0 | ((lba >> 24) & 0x0F);

    outb(0x1F6, drive_head);

    outb(0x1F2, 1);                  // Читать 1 сектор
    outb(0x1F3, (U8)lba);            // Младшая часть lba
    outb(0x1F4, (U8)(lba >> 8));     // Средняя часть lba
    outb(0x1F5, (U8)(lba >> 16));    // Старшая часть lba 
    outb(ATA_PRIMARY_STATUS, READ);  // Читать
    
    while ((inb(0x1F7) & (BSY | DRQ)) != DRQ);

    for(U32 i = 0; i < 256; i++) {
        word[i] = inw(0x1F0);
    }

    while (inb(0x1F7) & BSY);
}

U8 ata_write_sector(U32 lba, U16 word[256]) {
    U8 drive_head = 0xE0 | ((lba >> 24) & 0x0F);

    outb(0x1F6, drive_head);
    outb(0x1F2, 1);                         // Писать 1 сектор
    outb(0x1F3, (U8)lba);                   // Младшая часть lba
    outb(0x1F4, (U8)(lba >> 8));            // Средняя часть lba
    outb(0x1F5, (U8)(lba >> 16));           // Старшая часть lba 
    outb(ATA_PRIMARY_STATUS, WRITE);        // Писать

    while ((inb(0x1F7) & (BSY | DRQ)) != DRQ);

    for(U32 i = 0; i < 256; i++) {
        outw(0x1F0, word[i]);
    }

    while (inb(0x1F7) & BSY);
}

void ata_disk_handler() {
    return;
}

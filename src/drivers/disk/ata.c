#include <drivers/disk/ata.h>
#include <drivers/video/video.h>
#include <cpu/io.h>

#define BSY                     0x80
#define ERR                      0x1
#define DRQ                      0x8
#define DRIVE                   0xE0 
#define IDENTIFY                0xEC

#define ATA_PRIMARY_DATA       0x1F0
#define ATA_PRIMARY_STATUS     0x1F7
#define ATA_PRIMARY_ALT_STATUS 0x3F6

#define BUFFER_SIZE              256

#define SUCCESS                    0
#define DISK_NOT_FOUND             1
#define DISK_DONT_SUPPORT_PATA     2
#define DISK_ERROR                 3
#define DISK_TIMEOUT               4

#define READ                    0x20
#define WRITE                   0x30

U32 init_ata(U16 info[256]) {
    /* IDENTIFY */
    outb(disk_bar0 + 6, DRIVE);
    
    /* Задержка 400 нс */
    inb(disk_bar0 + 7);
    inb(disk_bar0 + 7);
    inb(disk_bar0 + 7);
    inb(disk_bar0 + 7);

    outb(disk_bar0 + 7, IDENTIFY);

    U8 exit_status = SUCCESS;
   
    /* Если 0x1F7 установлен на ноль, то диска не существует */
    if(inb(disk_bar0 + 7) == 0xFF) {
        exit_status = DISK_NOT_FOUND;
        goto exit; 
    }

    /* Ждем пока BSY установится на ноль */
    volatile U32 timeout = 10000 ;
    while((inb(disk_bar0 + 7) & BSY) && --timeout > 0) {
        asm volatile("outb %%al, $0x80" : : "a"(0)); 
    }
    if(timeout <= 0) return DISK_TIMEOUT;

    /* Если 0x1F4 и 0x1F5 равны нулю, то диск не поддерживает PATA */
    if(inb(disk_bar0 + 4) != 0 && inb(disk_bar0 + 5) != 0) {
        exit_status = DISK_DONT_SUPPORT_PATA;
        goto exit; 
    }

    U8 status;

    /* Ждем 1 в DRQ если успешно, или 1 в ERR в случаи ошибки */
    timeout = 10000 ;
    while(--timeout > 0) {
        status = inb(disk_bar0 + 7);
        if(status & DRQ) break;
        if(status & ERR) { exit_status = DISK_ERROR; goto exit; }
        asm volatile("outb %%al, $0x80" : : "a"(0));
    }
    if(timeout <= 0) return DISK_TIMEOUT;
    
    /* Читаем данные о диске из 0x1F0 в буффер */
    for(U32 i = 0; i < BUFFER_SIZE; i++) {
        info[i] = inw(disk_bar0);
    } 

exit:
    return exit_status;
}

U8 ata_read_sector(U32 lba, U16 word[256]) {
    // Установить устройство
    U8 drive_head = 0xE0 | ((lba >> 24) & 0x0F);

    outb(disk_bar0 + 6, drive_head);

    outb(disk_bar0 + 2, 1);                  // Читать 1 сектор
    outb(disk_bar0 + 3, (U8)lba);            // Младшая часть lba
    outb(disk_bar0 + 4, (U8)(lba >> 8));     // Средняя часть lba
    outb(disk_bar0 + 5, (U8)(lba >> 16));    // Старшая часть lba 
    outb(ATA_PRIMARY_STATUS, READ);  // Читать
    
    volatile U32 timeout = 5000000;
    while (((inb(disk_bar0 + 7) & (BSY | DRQ)) != DRQ) && --timeout > 0) {
        asm volatile("outb %%al, $0x80" : : "a"(0));
    }
    if(timeout <= 0) return DISK_TIMEOUT;

    for(U32 i = 0; i < 256; i++) {
        word[i] = inw(0x1F0);
    }

    timeout = 5000000;
    while ((inb(disk_bar0 + 7) & BSY) && --timeout > 0) {
        asm volatile("outb %%al, $0x80" : : "a"(0));
    }
    if(timeout <= 0) return DISK_TIMEOUT;

    return SUCCESS;
}

U8 ata_write_sector(U32 lba, U16 word[256]) {
    U8 drive_head = 0xE0 | ((lba >> 24) & 0x0F);

    outb(disk_bar0 + 6, drive_head);
    outb(disk_bar0 + 2, 1);                         // Писать 1 сектор
    outb(disk_bar0 + 3, (U8)lba);                   // Младшая часть lba
    outb(disk_bar0 + 4, (U8)(lba >> 8));            // Средняя часть lba
    outb(disk_bar0 + 5, (U8)(lba >> 16));           // Старшая часть lba 
    outb(ATA_PRIMARY_STATUS, WRITE);        // Писать

    volatile U32 timeout = 5000000;
    while (((inb(disk_bar0 + 7) & (BSY | DRQ)) != DRQ) && --timeout > 0) {
        asm volatile("outb %%al, $0x80" : : "a"(0));
    }
    if(timeout <= 0) return DISK_TIMEOUT;

    for(U32 i = 0; i < 256; i++) {
        outw(0x1F0, word[i]);
    }

    timeout = 5000000;
    while ((inb(disk_bar0 + 7) & BSY) && --timeout > 0) {
        asm volatile("outb %%al, $0x80" : : "a"(0));
    }
    if(timeout <= 0) return DISK_TIMEOUT;
}

void ata_disk_handler() {
    return;
}

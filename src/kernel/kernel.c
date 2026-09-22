#include <drivers/video/video.h>
#include <drivers/timer/timer.h>
#include <drivers/keyboard/keyboard.h>
#include <drivers/disk/disk.h>
#include <drivers/pci/pci.h>
#include <cpu/cpu.h>
#include <cpu/idt.h>
#include <cpu/pic.h>
#include <cpu/io.h>
#include <fs/fs.h>
#include <kernel/program.h>
#include <kernel/syscall.h>
#include <kernel/services.h>
#include <kernel/panic.h>

#define BOOT_SECTOR         0
#define BOOT_INFO_ADRESS    0x1000      // Сюда загрузчик поместит таблицу BOOT_INFO

extern void isr8();
extern void isr32();
extern void isr33();
extern void isr80();
extern void isr46();

services service;

void kmain() {
    idt_load();                     // Загрузить IDT
       
    idt_set(0x08, 0x08, 0x8E, (uint32_t)isr8); 
    idt_set(0x20, 0x08, 0x8E, (uint32_t)isr32); 
    idt_set(0x21, 0x08, 0x8E, (uint32_t)isr33); 
    idt_set(0x2E, 0x08, 0x8E, (uint32_t)isr46);
    idt_set(0x80, 0x08, 0x8E, (uint32_t)isr80);

    pic_remap();                    // Установка PIC
    pic_irq_mask(0x21, 0b11111000); // Включить IRQ
    pic_irq_mask(0xA1, 0b10111111); // PATA включить

    sti();

    pci_scan_bus0();

    init_memory();                  // Инициализация памяти
    service.memory->create_heap();  // Создание кучи
   
    init_keyboard();                // Инициализация клавиатуры              

    init_vga(VGA_640_480);          // Инициализация vga             
    
    video->kprintf("Kernel v 0.0.2\n");

    uint16_t* disk_info = service.memory->malloc(512);
    uint32_t disk_status = disk_init(disk_info);

    if(disk_status == SUCCES_INIT_DISK) {
        video->kprintf("[  %f10OK%f15  ] ATA driver initialized successfully\n");
    }else {
        if(disk_status == DISK_NOT_FOUND) {
            video->kprintf("[ %f12FAIL%f15 ] ATA driver init: Disk not found\n");
        }
        else if(disk_status == DISK_DONT_SUPPORT_PATA) {
            video->kprintf("[ %f12FAIL%f15 ] ATA driver init: Unsupported device type\n");
        }else if(disk_status == DISK_ERROR) {
            video->kprintf("[ %f12FAIL%f15 ] ATA driver init: ATA driver init: Disk I/O error\n");
        }else if(disk_status == DISK_TIMEOUT) {
            video->kprintf("[ %f12FAIL%f15 ] ATA driver init: ATA driver init: Disk timeout\n");
        }
    }

    u32 fs_status = init_fs();    

    init_timer(100);

    u8* cpuid = service.memory->malloc(48);
    service.memory->memcpy(0x1008, cpuid, 48);
    video->kprintf("[ INFO ] CPU: %s\n", cpuid);

    u16* ahci_read_status = 0;

    u8 word[512];
    disk->read_sector(0, word);

    ahci_read_status = &(word[510]);

    if(*ahci_read_status == 0xAA55) {
        video->kprintf("AHCI read succes");
    }else {
        video->kprintf("AHCI read fail");
    }

    video->kprintf("\nFS Type: %d\n", word[462]);

    //fs->open("INIT    BIN");

	for(;;) {
        halt();
    }
}
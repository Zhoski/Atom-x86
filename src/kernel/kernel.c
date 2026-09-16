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

    uint16_t* disk_info = service.memory->malloc(512);
    uint32_t disk_status = disk_init(disk_info);

    if(disk_status == SUCCES_INIT_DISK) {
        video->write_string("[  ");
        video->terminal_fg_vbe_set(10);
        video->write_string("OK");
        video->terminal_fg_vbe_set(15);
        video->write_string("  ] Ata driver init\n[ INFO ] Disk: ");
        for(uint32_t idx = 27;idx < 46;idx++) {
            video->write_char((U8*)(((disk_info[idx] >> 8)) & 0xFF));
            video->write_char((U8*)(disk_info[idx] & 0xFF));
        }
        video->write_char('\n');
    }else {
        if(disk_status == DISK_NOT_FOUND) {
            video->write_string("[ ");
            video->terminal_fg_vbe_set(12);
            video->write_string("FAIL");
            video->terminal_fg_vbe_set(15);
            video->write_string(" ] Ata driver ini: Disk Not Found\n");
        }
        else if(disk_status == DISK_DONT_SUPPORT_PATA) {
            video->write_string("[ ");
            video->terminal_fg_vbe_set(12);
            video->write_string("FAIL");
            video->terminal_fg_vbe_set(15);
            video->write_string(" ] Ata driver init: Disk Dont Support PATA\n");
        }else if(disk_status == DISK_ERROR) {
            video->write_string("[ ");
            video->terminal_fg_vbe_set(12);
            video->write_string("FAIL");
            video->terminal_fg_vbe_set(15);
            video->write_string(" ] Ata driver init: Disk Error\n");
        }else if(disk_status == DISK_TIMEOUT) {
            video->write_string("[ ");
            video->terminal_fg_vbe_set(12);
            video->write_string("FAIL");
            video->terminal_fg_vbe_set(15);
            video->write_string(" ] Ata driver init: Disk Timeout\n");
        }
    }

    U32 fs_status = init_fs();    

    if(fs_status == 0) {
        video->write_string("[  ");
        video->terminal_fg_vbe_set(10);
        video->write_string("OK");
        video->terminal_fg_vbe_set(15);
        video->write_string("  ] FS driver init\n");
    }else if(fs_status == DISK_TIMEOUT) {
        video->write_string("[ ");
        video->terminal_fg_vbe_set(12);
        video->write_string("FAIL");
        video->terminal_fg_vbe_set(15);
        video->write_string(" ] FS driver init: Disk TimeOut\n");
    }

    init_timer(100);

    U8* cpuid = service.memory->malloc(48);
    service.memory->memcpy(0x1008, cpuid, 48);
    video->write_string("[ INFO ] CPU: ");
    video->write_string(cpuid);
    video->write_char("\n");

    //fs->open("INIT    BIN");

	for(;;) {
        halt();
    }
}
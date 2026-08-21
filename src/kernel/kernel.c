#include "../drivers/video/video.h"
#include "../drivers/Keyboard/keyboard.h"
#include "../drivers/disk/disk.h"
#include "../cpu/idt.h"
#include "../cpu/PIC.h"
#include "services/memory/program.h"
#include "services/syscall/syscall.h"
#include "../drivers/fs/fs.h"
#include "services/services.h"
#include "port/io.h"
#include "services/panic/panic.h"
#include <stdint.h>

#define BOOT_SECTOR         0
#define BOOT_INFO_ADRESS    0x1000      // Сюда загрузчик поместит таблицу BOOT_INFO

extern void isr8();
extern void isr33();
extern void isr80();
extern void isr46();

services service;

void kmain() {
    idt_load();                     // Загрузить IDT
                                    
    idt_set(0x21, 0x08, 0x8E, (uint32_t)isr33); 
    idt_set(0x2E, 0x08, 0x8E, (uint32_t)isr46);
    idt_set(0x80, 0x08, 0x8E, (uint32_t)isr80);

    pic_remap();                    // Установка PIC
    pic_irq_mask(0x21, 0b11111001); // Включить IRQ
    pic_irq_mask(0xA1, 0b10111111); // PATA включить

    init_memory();                  // Инициализация памяти
    service.memory->create_heap();  // Создание кучи
   
    init_keyboard();                // Инициализация клавиатуры              

    init_vga(VGA_640_480);          // Инициализация vga                       

    uint16_t* disk_info = service.memory->malloc(512);
    disk_init(disk_info);
    service.memory->free(disk_info);
    init_fs();    

    free(disk_info);

    asm("sti");

    fs->open("NOTEPAD BIN");
    //fs->open("INIT    BIN");

	for(;;) {
        asm("hlt");
    }
}
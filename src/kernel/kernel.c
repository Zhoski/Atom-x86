#include "../drivers/video/video.h"
#include "../drivers/Keyboard/keyboard.h"
#include "../drivers/disk/pata.h"
#include "../cpu/idt.h"
#include "../cpu/PIC.h"
#include "services/memory/allocate.h"
#include "services/memory/program.h"
#include "services/syscall/syscall.h"
#include "../drivers/fs/fat16.h"
#include "services/services.h"
#include "port/io.h"
#include <stdint.h>

#define BOOT_SECTOR         0
#define BOOT_INFO_ADRESS    0x1000      // Сюда загрузчик поместит таблицу BOOT_INFO

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
    asm("sti");                     // Включить перывания	
    
    uint16_t info[256];
    init_pata(info); 
   
    init_keyboard();                // Инициализация клавиатуры
    init_memory();                  // Инициализация памяти
    init_allocate();                // Инициализация алокатора
    //init_config();                // Инициализация конфигов 

    uint8_t graphics_mode = *(uint8_t*)(BOOT_INFO_ADRESS + 0xE);
    init_vga(graphics_mode);                  // Инициализация vga
                                              
    uint16_t bootSector[256];
    disk.read_sector(BOOT_SECTOR, bootSector);
    init_fat16(bootSector);

    /*uint8_t status = find_file("SHELL   BIN"); 
    
    if(status == 1) {
        video.write_string("File found\n",0,255,0);
    }else {
        video.write_string("File not found\n",255,0,0);
    }*/

    open("SHELL   BIN"); 

    //open("Shell.bin");

    //program_spawn(0x2000);

	for(;;) {
        asm("hlt");
    }
}

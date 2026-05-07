#include "../drivers/VGA/vga.h"
#include "../drivers/Keyboard/keyboard.h"
#include "../drivers/disk/pata.h"
#include "../cpu/idt.h"
#include "../cpu/PIC.h"
#include "services/memory/allocate.h"
#include "services/memory/program.h"
#include "services/syscall/syscall.h"
#include "services/services.h"
#include "port/io.h"
#include "config/config.h"
#include <stdint.h>

extern void isr33();
extern void isr80();
extern void isr46();

uint8_t user_name[32];
uint8_t user_pass[32];

services service;

void kmain() {
    idt_load();                     // Загрузить IDT
                                    
    idt_set(0x21, 0x08, 0x8E, (uint32_t)isr33); 
    idt_set(0x2E, 0x08, 0x8E, (uint32_t)isr46);
    idt_set(0x80, 0x08, 0x8E, (uint32_t)isr80);

    pic_remap();                    // Установка PIC
    pic_irq_mask(0x21, 0b11111001);    // Включить IRQ
    pic_irq_mask(0xA1, 0b10111111);    // PATA включить
    asm("sti");                     // Включить перывания	
                                
    init_pata();
   
    init_keyboard();                // Инициализация клавиатуры
    init_vga();                     // Инициализация vga
    init_memory();                  // Инициализация памяти
    init_allocate();                // Инициализация алокатора
    init_config();                  // Инициализация конфигов  

    service.vga->clear();			// Очистка  
    

    service.vga->set_attribute(VGA_COLOR_BLACK, VGA_COLOR_WHITE);
    program_spawn(0x2000);

	for(;;) {
        asm("hlt");
    }
}

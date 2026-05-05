#include "../drivers/VGA/vga.h"
#include "../drivers/Keyboard/keyboard.h"
#include "../cpu/idt.h"
#include "../cpu/PIC.h"
#include "services/Memory/allocate.h"
#include "services/Memory/program.h"
#include "services/syscall/syscall.h"
#include "services/services.h"
#include "port/io.h"
#include "config/config.h"
#include <stdint.h>

extern void isr33();
extern void isr80();

uint8_t user_name[32];
uint8_t user_pass[32];

services service;

void kmain() {
    idt_load();                     // Загрузить IDT
                                    
    idt_set(0x21, 0x08, 0x8E, (uint32_t)isr33);  
    idt_set(0x80, 0x08, 0x8E, (uint32_t)isr80);

    pic_remap();                    // Установка PIC
    pic_irq_mask(0x21, 0b11111101); // Включить только IRQ1
    asm("sti");                     // Включить перывания	
   
    init_keyboard();                // Инициализация клавиатуры
    init_vga();                     // Инициализация vga
    init_memory();
    init_config();                  // Инициализация конфигов
    
    service.vga->clear();			// Очистка
    service.vga->set_attribute(VGA_COLOR_BLACK, VGA_COLOR_WHITE);
    program_spawn(0x2000);

	for(;;) {
        asm("hlt");
    }
}

#include "../drivers/vga.h"
#include "../drivers/keyboard.h"
#include "../cpu/idt.h"
#include "../cpu/PIC.h"
#include "services/Memory/memory_map.h"
#include "services/Memory/memory.h"
#include <stdint.h>

extern void isr33();

void kmain() {
    idt_load();                     // Загрузить IDT
    idt_set(33, 0x08, 0x8E, (uint32_t)isr33);                                   
    pic_remap();                    // Установка PIC
    pic_irq_mask(0x21, 0b11111101); // Включить только IRQ1
    asm("sti" :: );                 // Включить перывания
	
	clear_screen();			// Очистка
    
    vga_set_attribute(VGA_COLOR_BLACK, VGA_COLOR_WHITE);
    kwrite_string("Hello, World!"); 

    //uint8_t* config = (uint8_t*)0x1000;
    //putchar(config[2]);
    uint32_t adres = malloc_page(); 
    // Скопировать терминал из 0x2000 в 0x100000
    memcpy(0x2000, adres, 512);
    asm volatile (
        "pushl $0x08 \n\t"   
        "pushl %0   \n\t"    
        "lretl"                    
        : 
        : "r" (adres)       
    );

	for(;;) {
        asm("hlt"::);
    }
}

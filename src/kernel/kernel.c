#include "../drivers/vga.h"
#include "../cpu/idt.h"
#include "../cpu/PIC.h"
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
	for(;;);
}

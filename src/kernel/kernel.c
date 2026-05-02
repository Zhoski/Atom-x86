#include "../drivers/VGA/vga.h"
#include "../drivers/Keyboard/keyboard.h"
#include "../cpu/idt.h"
#include "../cpu/PIC.h"
#include "services/Memory/memory_map.h"
#include "services/Memory/memory.h"
#include "services/Memory/program.h"
#include "services/syscall/syscall.h"
#include "config/config.h"
#include <stdint.h>

extern void isr33();
extern void isr80();

uint8_t user_name[32];
uint8_t user_pass[32];

void kmain() {
    idt_load();                     // Загрузить IDT
    idt_set(0x21, 0x08, 0x8E, (uint32_t)isr33);  
    idt_set(0x80, 0x08, 0x8E, (uint32_t)isr80);

    pic_remap();                    // Установка PIC
    pic_irq_mask(0x21, 0b11111101); // Включить только IRQ1
    asm("sti");                     // Включить перывания 
	clear_screen();			        // Очистка
    
    init_config();                  // Чтение конфигов
    vga_set_attribute(VGA_COLOR_BLACK, VGA_COLOR_WHITE);
    //kwrite_string("Hello, World!\n");  
    program_spawn(0x2000);  // Адрес терминала в оперативке
    
    //uint8_t* config = (uint8_t*)0x1000;
    //putchar(config[2]);
    //uint32_t adres = malloc_page();     // Память для терминада
    //uint32_t stack = malloc_stack();    // Стек
    // Скопировать терминал из 0x2000 в 0x100000
    //memcpy((int*)0x2000, adres, 512); 
    //process_spawn(adres, stack);
    //program_execute(adres, stack+0x2000);
    //asm volatile (
    //    "pushl $0x08 \n\t"   
    //    "pushl %0   \n\t"    
    //    "lretl"                    
    //    : 
    //    : "r" (adres)       
    //);

	for(;;) {
        asm("hlt");
    }
}

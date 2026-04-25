#include "../drivers/vga.h"
#include <stdint.h>

inline void outb(uint16_t port, uint8_t data) {
    asm("out %0, %1" :: "a"(port), "d"(data));
}

inline uint32_t in(uint16_t port) {
    uint32_t data;
    asm volatile ("inl %1, %0" : "=a"(data) : "d"(port));
    return data;
}

void kmain() {
	uint8_t* vga_video = (uint8_t*)0xB8000;
	
	clear_screen();			// Очистка
	
    putchar('A');	
    
    vga_set_attribute(VGA_COLOR_BLUE, VGA_COLOR_WHITE);
    kwrite_string("Zvor");
	for(;;);
}

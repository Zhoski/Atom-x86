#include "../drivers/vga.h"
#include <stdint.h>

void kmain() {
	uint8_t* vga_video = (uint8_t*)0xB8000;
	vga_video[0] = '!';
	vga_video[1] = 0x07;
	clear_screen();			// Очистка
	putchar('A');	
    	kwrite_string("Zvor");
	for(;;);
}

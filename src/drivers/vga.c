#include "vga.h"

uint16_t* vga_video = (uint16_t*)VGA_MEMORY;
uint8_t terminal_row = 0;
uint8_t terminal_column = 0;

void clear_screen() {
    	terminal_row = 0;
    	terminal_column = 0;
    
    	uint16_t blank = (uint8_t)0x07 << 8 | ' ';

    	for (uint16_t index = 0; index < VGA_HEIGHT * VGA_WIDTH; index++) {
        	vga_video[index] = blank;
	}
}

void putchar(const char data) {
	const uint16_t index = (terminal_row * VGA_WIDTH + terminal_column);
	uint16_t blank = (uint8_t)0x07 << 8 | data;
	vga_video[index] = blank;
	terminal_column++;
}

void kwrite_string(const char* data) {
	uint8_t size = 0;
	while(data[size++] != '\0');

	for(uint16_t i = 0;i < size;i++) {
		putchar(data[i]);
	}
}

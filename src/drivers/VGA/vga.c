#include "vga.h"

uint16_t* vga_video = (uint16_t*)VGA_MEMORY;
uint8_t terminal_row = 0;
uint8_t terminal_column = 0;
uint8_t terminal_color = 0x07;

static inline void outb(uint16_t port, uint8_t data) {
    asm volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

void updateCursorPosition(uint8_t x, uint8_t y) {
    uint16_t position = (terminal_row * 80) + terminal_column;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(position & 0xFF)); 
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((position >> 8) & 0xFF));
}

inline uint8_t vga_entry_color(uint8_t bg, uint8_t fg) {
    return fg | bg << 4;
}

void vga_set_attribute(uint8_t bg, uint8_t fg) {
    terminal_color = vga_entry_color(bg, fg);
}

void clear_screen() {
    	terminal_row = 0;
    	terminal_column = 0;
    
    	uint16_t blank = terminal_color << 8 | ' ';

    	for (uint16_t index = 0; index < VGA_HEIGHT * VGA_WIDTH; index++) {
        	vga_video[index] = blank;
	}
}

void putchar(const char data) {
    if(data == 0) {
        return;
    }
    if(data == 10) {
        terminal_row++;
        terminal_column = 0;
    }else {
        const uint16_t index = (terminal_row * VGA_WIDTH + terminal_column);
	    uint16_t blank = terminal_color << 8 | data;
	    vga_video[index] = blank;
	    terminal_column++; 
    }

    updateCursorPosition(terminal_row, terminal_column);
}

void kwrite_string(const char* data) {
	uint8_t size = 0;
	while(data[size++] != '\0');

	for(uint16_t i = 0;i < size - 1;i++) {
	   putchar(data[i]); 
	}
}

void kwrite_int(int x) {
    if(x == 0) {
        putchar('0');
        return;
    }
    if(x < 0) {
        putchar('-');
    }
    char buffer[12];
    int i = 0;
    while (x > 0) {
        buffer[i++] = (x % 10) + '0';
        x /= 10;
    }

    while (i > 0) {
        putchar(buffer[--i]);
    }
}

void kwrite_hex(int x, int t) {
    // t это сколько знаков
    char *hex_chars = "0123456789ABCDEF";
    
    for (int i = t; i >= 0; i--) {
        char c = hex_chars[(x >> (i * 4)) & 0xF];
        putchar(c); 
    }
}


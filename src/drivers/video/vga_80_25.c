#include "vga_80_25.h"

#define VGA_80_25_MEMORY 0xB8000
#define VGA_80_25_HEIGHT 25
#define VGA_80_25_WIDTH  80

uint16_t* vga_video = (uint16_t*)VGA_80_25_MEMORY;
uint8_t terminal_row = 0;
uint8_t terminal_column = 0;
uint8_t terminal_color = 0x07;

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

void clear_screen(uint8_t r, uint8_t g, uint8_t b) {
    terminal_row = 0;
    terminal_column = 0;
    
    uint16_t blank = terminal_color << 8 | ' ';

    for (uint16_t index = 0; index < VGA_80_25_HEIGHT * VGA_80_25_WIDTH; index++) {
        vga_video[index] = blank;
	}
}

void vga_80_25_write_char(const uint8_t c, uint8_t color) {
    if(c == 0) {
        return;
    }
    if(c == '\n') {
        terminal_row++;
        terminal_column = 0;
    }else {
        const uint16_t index = (terminal_row * VGA_80_25_WIDTH + terminal_column);
	    uint16_t blank = terminal_color << 8 | c;
	    vga_video[index] = blank;
	    terminal_column++; 
    }

    updateCursorPosition(terminal_row, terminal_column);
}

void vga_80_25_write_string(const char* s, uint8_t color) {    
    while(*s) {
        vga_80_25_write_char(*s,color);
        s++;
    }
}
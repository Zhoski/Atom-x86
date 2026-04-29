#include "vga.h"

uint16_t* vga_video = (uint16_t*)VGA_MEMORY;
uint8_t terminal_row = 0;
uint8_t terminal_column = 0;
uint8_t terminal_color = 0x07;

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
    if(data == '\n') {
        terminal_row++;
        terminal_column = 0;
    }else {
        const uint16_t index = (terminal_row * VGA_WIDTH + terminal_column);
	    uint16_t blank = terminal_color << 8 | data;
	    vga_video[index] = blank;
	    terminal_column++; 
    }
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

void sys_write(uint8_t* str, uint32_t len) { 
    kwrite_string("[DEBUG-START]\n");
    kwrite_string("len: "); kwrite_int(len);
    kwrite_string("\n");
    for (uint32_t i = 0; i < len; i++) {
        if (str[i] == 0) putchar('?');
        else putchar(str[i]);
    }
    kwrite_string("\n[DEBUG-END]\n");
}

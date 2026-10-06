#include <drivers/video/vga_80_25.h>
#include <cpu/io.h>

#define VGA_80_25_MEMORY 0xB8000
#define VGA_80_25_HEIGHT 25
#define VGA_80_25_WIDTH  80

u16* vga_video = (U16*)VGA_80_25_MEMORY;
u8 terminal_row = 0;
u8 terminal_column = 0;
u8 terminal_color = 0x07;

void updateCursorPosition(u8 x, u8 y) {
    u16 position = (terminal_row * 80) + terminal_column;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (u8)(position & 0xFF)); 
    outb(0x3D4, 0x0E);
    outb(0x3D5, (u8)((position >> 8) & 0xFF));
}

inline U8 vga_entry_color(u8 bg, u8 fg) {
    return fg | bg << 4;
}

void vga_set_attribute(U8 bg, U8 fg) {
    terminal_color = vga_entry_color(bg, fg);
}

void clear_screen(u8 r, u8 g, u8 b) {
    terminal_row = 0;
    terminal_column = 0;
    
    u16 blank = terminal_color << 8 | ' ';

    for (u16 index = 0; index < VGA_80_25_HEIGHT * VGA_80_25_WIDTH; index++) {
        vga_video[index] = blank;
	}
}

void vga_80_25_write_char(const u8 c) {
    if(c == 0) {
        return;
    }
    if(c == '\n') {
        terminal_row++;
        terminal_column = 0;
    }else {
        const u16 index = (terminal_row * VGA_80_25_WIDTH + terminal_column);
	    u16 blank = terminal_color << 8 | c;
	    vga_video[index] = blank;
	    terminal_column++; 
    }

    updateCursorPosition(terminal_row, terminal_column);
}

void vga_80_25_write_string(const u8* s) {    
    while(*s) {
        vga_80_25_write_char(*s);
        s++;
    }
}

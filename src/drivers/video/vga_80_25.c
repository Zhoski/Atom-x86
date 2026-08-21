#include <drivers/video/vga_80_25.h>

#define VGA_80_25_MEMORY 0xB8000
#define VGA_80_25_HEIGHT 25
#define VGA_80_25_WIDTH  80

U16* vga_video = (U16*)VGA_80_25_MEMORY;
U8 terminal_row = 0;
U8 terminal_column = 0;
U8 terminal_color = 0x07;

void updateCursorPosition(U8 x, U8 y) {
    U16 position = (terminal_row * 80) + terminal_column;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (U8)(position & 0xFF)); 
    outb(0x3D4, 0x0E);
    outb(0x3D5, (U8)((position >> 8) & 0xFF));
}

inline U8 vga_entry_color(U8 bg, U8 fg) {
    return fg | bg << 4;
}

void vga_set_attribute(U8 bg, U8 fg) {
    terminal_color = vga_entry_color(bg, fg);
}

void clear_screen(U8 r, U8 g, U8 b) {
    terminal_row = 0;
    terminal_column = 0;
    
    U16 blank = terminal_color << 8 | ' ';

    for (U16 index = 0; index < VGA_80_25_HEIGHT * VGA_80_25_WIDTH; index++) {
        vga_video[index] = blank;
	}
}

void vga_80_25_write_char(const U8 c, U8 color) {
    if(c == 0) {
        return;
    }
    if(c == '\n') {
        terminal_row++;
        terminal_column = 0;
    }else {
        const U16 index = (terminal_row * VGA_80_25_WIDTH + terminal_column);
	    U16 blank = terminal_color << 8 | c;
	    vga_video[index] = blank;
	    terminal_column++; 
    }

    updateCursorPosition(terminal_row, terminal_column);
}

void vga_80_25_write_string(const char* s, U8 color) {    
    while(*s) {
        vga_80_25_write_char(*s,color);
        s++;
    }
}

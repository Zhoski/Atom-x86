#include "video.h"
#include "../../kernel/port/io.h"
#include "../../kernel/device/device.h"
#include "../../kernel/services/services.h"

#define FONT8x16_IMPLEMENTATION
#include "../../font/font8x16.h"

#define INDEX_REGISTER          0x3CE
#define DATA_REGISTER           0x3CF

#define GRAPHICS_MODE            0x05
#define BIT_MASK                 0x08

#define VGA_640_480_WIDTH         640
#define VGA_640_480_HEIGHT        480

#define VESA_1024_768_WIDTH      1024
#define VESA_1024_768_HEIGHT      768

enum graphics_mode {
    VGA_80_25 = 0x01,          // VGA текстовый 80x25 16 цветов
    VGA_640_480 = 0x02,        // VGA графический 640x480 16 цветов
    VBE_1024_768 = 0x03,       // VBE графический 1024x768 16M цветов
};

VideoDriver video;

uint16_t* vga_video = (uint16_t*)VGA_MEMORY;
uint8_t terminal_row = 0;
uint8_t terminal_column = 0;
uint8_t terminal_color = 0x07;
uint32_t screen_x_off = 0;
uint32_t screen_y_off = 0;
uint8_t* vbe_lfb = 0;

uint8_t terminal_fg_vbe = 15;   // Белый
uint8_t terminal_bg_vbe = 0;    // Черный

void init_vga(uint8_t mode) {
    if(mode == VGA_640_480) {
        video.write_string = &vga_640_480_draw_string;
        video.write_char = &vga_640_480_draw_char;
        video.clear_screen = &vga_640_480_clear_screen;
        video.terminal_bg_vbe_set = &terminal_bg_vbe_set;
        video.terminal_fg_vbe_set = &terminal_fg_vbe_set;
    }else if(mode == VGA_80_25) {
        video.write_string = &vga_80_25_write_string;
        video.write_char = &vga_80_25_write_char;
        video.clear_screen = &clear_screen;
    }
};

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

    for (uint16_t index = 0; index < VGA_HEIGHT * VGA_WIDTH; index++) {
        vga_video[index] = blank;
	}
}

void vga_80_25_write_char(const uint8_t c, uint8_t r, uint8_t g, uint8_t b) {
    if(c == 0) {
        return;
    }
    if(c == '\n') {
        terminal_row++;
        terminal_column = 0;
    }else {
        const uint16_t index = (terminal_row * VGA_WIDTH + terminal_column);
	    uint16_t blank = terminal_color << 8 | c;
	    vga_video[index] = blank;
	    terminal_column++; 
    }

    updateCursorPosition(terminal_row, terminal_column);
}

void vga_80_25_write_string(const char* s, uint8_t r, uint8_t g, uint8_t b) {    
    while(*s) {
        vga_80_25_write_char(*s,r,g,b);
        s++;
    }
}


void vga_640_480_clear_screen(uint8_t color) {
    screen_x_off = 0;
    screen_y_off = 0;

    service.memory->memset(VGA_640_480_MEMORY, color, 640 * 80);  
}

void vga_640_480_write_row(uint32_t x, uint32_t y, uint8_t b, uint8_t fg, uint8_t bg) {
    uint32_t offset = ((y << 6) + (y << 4)) + (x >> 3);
    
    volatile uint8_t *address = VGA_640_480_MEMORY + offset;

    outb(INDEX_REGISTER, GRAPHICS_MODE);
    outb(DATA_REGISTER, 0x00);

    outb(0x03C4, 0x02);
    outb(0x03C5, 0x0F);

    outb(INDEX_REGISTER, 0x00);
    outb(DATA_REGISTER, fg);

    outb(0x03CE, 0x01); 
    outb(0x03CF, 0x0F);

    outb(INDEX_REGISTER, 0x08);
    outb(DATA_REGISTER, b);

    volatile uint8_t dummy = *address;
    
    *address = bg;
}

void vga_640_480_putpixel(uint32_t x, uint32_t y, uint8_t color) { 
    uint32_t offset = ((y << 6) + (y << 4)) + (x >> 3);
    uint8_t bit_mask = 0x80 >> (x & 7);
    
    volatile uint8_t *address = VGA_640_480_MEMORY + offset;

    outb(INDEX_REGISTER, GRAPHICS_MODE);
    outb(DATA_REGISTER, 0x02);
    outb(INDEX_REGISTER, BIT_MASK);
    outb(DATA_REGISTER, bit_mask);

    uint8_t dummy = *address;
    
    *address = color;
}

void vga_640_480_scroll() {
    uint32_t offset = 80 + VGA_640_480_MEMORY;
    uint32_t new_offset = 0 + VGA_640_480_MEMORY;

    for(uint32_t i = 0; i < 480;i++) {
        service.memory->memcpy(offset, new_offset, 80);
        offset += 80;
        new_offset += 80;
    }

    service.memory->memset(offset, terminal_bg_vbe, 80);
}

void vga_640_480_draw_char(uint8_t c) {
    if(c == '\n') {
        screen_x_off = 0;
        screen_y_off += 16;
        if(screen_y_off == 464) {
            for(uint32_t i = 0;i < 16;i++) {
                vga_640_480_scroll();
            }
            screen_y_off -= 16;
        }
        return;
    }
    for(uint32_t row = 0; row < 16;row++) {
        uint8_t row_byte = font8x16[c][row];
        vga_640_480_write_row(screen_x_off, screen_y_off + row, row_byte, terminal_fg_vbe, terminal_bg_vbe);
    }

    screen_x_off+=8;
}

void vga_640_480_draw_string(const uint8_t* s) {
    while(*s) {
        vga_640_480_draw_char(*s);
        s++; 
    }
}

void terminal_fg_vbe_set(uint8_t color) {
    terminal_fg_vbe = color;
}

void terminal_bg_vbe_set(uint8_t color) {
    terminal_bg_vbe = color;
}
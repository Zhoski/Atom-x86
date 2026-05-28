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

Video video;

uint16_t* vga_video = (uint16_t*)VGA_MEMORY;
uint8_t terminal_row = 0;
uint8_t terminal_column = 0;
uint8_t terminal_color = 0x07;
uint32_t screen_x_off = 0;
uint32_t screen_y_off = 0;
uint8_t* vbe_lfb = 0;

uint32_t terminal_fg_vbe = 0xFFFFFF00;   // Белый
uint32_t terminal_bg_vbe = 0x00000000;   // Черный

void init_vga(uint8_t mode) {
    if(mode == VBE_1024_768) {
        video.write_string = &vesa_1024_768_draw_string;
        video.write_char = &vesa_1024_768_draw_char;
        video.putpixel = &vesa_1024_768_putpixel;
        video.clear_screen = &vesa_1024_768_clear_screen;
        video.terminal_bg_vbe_set = &terminal_bg_vbe_set;
        video.terminal_fg_vbe_set = &terminal_fg_vbe_set;

        uint32_t lfb_adress = *(uint32_t*)(0x1000 + 0x0A);
        vbe_lfb = (uint8_t*)lfb_adress;

    }else if(mode == VGA_640_480) {
        video.write_string = &vga_640_480_draw_string;
        video.write_char = &vga_640_480_draw_char;
        video.putpixel = &vga_640_480_putpixel;
        video.clear_screen = &vga_640_480_clear_screen;
    }else if(mode == VGA_80_25) {
        video.write_string = &vga_80_25_write_string;
        video.write_char = &vga_80_25_write_char;
        video.clear_screen = &clear_screen;
        video.putpixel = 0;  
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


void vga_640_480_clear_screen(uint8_t r, uint8_t g, uint8_t b) {
     screen_x_off = 0;
     screen_y_off = 0;

     for(;screen_y_off < VGA_640_480_HEIGHT;screen_x_off++) {
        if(screen_x_off == VGA_640_480_WIDTH) {
            screen_x_off = 0;
            screen_y_off++;
        }
        vga_640_480_putpixel(screen_x_off, screen_y_off, r, g, b);
     }

    screen_x_off = 0;
    screen_y_off = 0;
}

void vga_640_480_putpixel(uint32_t x, uint32_t y, uint8_t color, uint8_t n, uint8_t n2) {
    uint32_t offset = (y * 80) + (x / 8);
    uint8_t bit_mask = 0x80 >> (x & 7);
    
    volatile uint8_t *address = VGA_640_480_MEMORY + offset;

    outb(INDEX_REGISTER, GRAPHICS_MODE);
    outb(DATA_REGISTER, 0x02);
    outb(INDEX_REGISTER, BIT_MASK);
    outb(DATA_REGISTER, bit_mask);

    uint8_t dummy = *address;
    
    *address = color;
}

void vga_640_480_draw_char(uint8_t c, uint8_t color, uint8_t n, uint8_t n2) {
    if(c == '\n') {
        screen_x_off = 0;
        screen_y_off += 16;
        return;
    }
    for (int i = 0; i < 16; i++) {
        uint8_t row_byte = font8x16[c][i];
        for (int j = 0; j < 8; j++) {
            uint8_t is_pixel_active = (row_byte >> (7 - j)) & 1; 
            if (is_pixel_active) {
                vga_640_480_putpixel(j + screen_x_off, i + screen_y_off, color,0,0); 
            }
        }
    }

    screen_x_off+=8;
}

void vga_640_480_draw_string(const uint8_t* s, uint8_t color, uint8_t n, uint8_t n2) {
    while(*s) {
        vga_640_480_draw_char(*s, color,0,0);
        s++; 
    }
}

void vga_640_480_draw_int(int x, uint8_t color, uint8_t n, uint8_t n2) {
    if(x == 0) {
        vga_640_480_draw_char('0',color,0,0);
        return;
    }
    if(x < 0) {
        vga_640_480_draw_char('-',color,0,0);
    }
    char buffer[12];
    int i = 0;
    while (x > 0) {
        buffer[i++] = (x % 10) + '0';
        x /= 10;
    }

    while (i > 0) {
        vga_640_480_draw_char(buffer[--i],color,0,0);
    }
}

// =================== VESA ====================

void terminal_fg_vbe_set(uint8_t r, uint8_t g, uint8_t b) {
    uint32_t color = 0x00000000;

    uint32_t _r = r;
    _r <<= 24;
    uint32_t _g = g;
    _g <<= 16;
    uint32_t _b = b;
    _b <<= 8;

    color |= _r;
    color |= _g;
    color |= _b;

    terminal_fg_vbe = color;
}

void terminal_bg_vbe_set(uint8_t r, uint8_t g, uint8_t b) {
    uint32_t color = 0x00000000;

    uint32_t _r = r;
    _r <<= 24;
    uint32_t _g = g;
    _g <<= 16;
    uint32_t _b = b;
    _b <<= 8;

    color |= _r;
    color |= _g;
    color |= _b;

    terminal_bg_vbe = color;
}

void vesa_1024_768_clear_screen(uint8_t r, uint8_t g, uint8_t b) {
    screen_x_off = 0;
    screen_y_off = 0;

    for(;screen_y_off < VESA_1024_768_HEIGHT;screen_x_off++) {
        if(screen_x_off == VESA_1024_768_WIDTH) {
            screen_x_off = 0;
            screen_y_off++;
        }
        vesa_1024_768_putpixel(screen_x_off, screen_y_off, r, g, b);
    }

    screen_x_off = 0;
    screen_y_off = 0;  
    
}

void vesa_1024_768_putpixel(uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b) {
    uint32_t pitch = 1024 * 3;
    uint32_t offset = (y * pitch) + (x * 3);

    vbe_lfb[offset]     = b; // Синий
    vbe_lfb[offset + 1] = g; // Зеленый
    vbe_lfb[offset + 2] = r; // Красный
}

void vesa_1024_768_scroll() { 
    uint32_t pitch = 3072;
    uint32_t new_y = 0, y = 1;
    uint32_t old_pos = (y * pitch);
    uint32_t new_pos = (new_y * pitch);
    while (y < VESA_1024_768_HEIGHT)
    {
        service.memory->memcpy((vbe_lfb+old_pos), (vbe_lfb+new_pos), pitch);

        y++;
        new_y++;

        old_pos = (y * pitch);
        new_pos = (new_y * pitch);
    }
    
    for(uint32_t x = 0; x < 1024;x++) {
        vesa_1024_768_putpixel(x, y, 0,0,0);
    }
    
}

void vesa_1024_768_draw_char(uint8_t c) {
    if(c == '\n') {
        screen_x_off = 0;
        screen_y_off += 16;
        if(screen_y_off >= 752) {
            for(uint32_t i = 0;i < 16;i++) {
                vesa_1024_768_scroll();
            }
            screen_y_off -= 16;
        }
        return;
    }

    if(c == "\"") {
        c = '"';
    }

    uint8_t fgR = (uint8_t)(terminal_fg_vbe >> 24);
    uint8_t fgG = (uint8_t)(terminal_fg_vbe >> 16);
    uint8_t fgB = (uint8_t)(terminal_fg_vbe >> 8);

    uint8_t bgR = (uint8_t)(terminal_bg_vbe >> 24);
    uint8_t bgG = (uint8_t)(terminal_bg_vbe >> 16);
    uint8_t bgB = (uint8_t)(terminal_bg_vbe >> 8);

    for (int i = 0; i < 16; i++) {
        
        uint8_t row_byte = font8x16[c][i]; 
        
        for (int j = 0; j < 8; j++) {
            
            uint8_t is_pixel_active = (row_byte >> (7 - j)) & 1; 

            if (is_pixel_active) {
                vesa_1024_768_putpixel(j + screen_x_off, i + screen_y_off,fgR,fgG,fgB); 
            } else {               
                vesa_1024_768_putpixel(j + screen_x_off, i + screen_y_off, bgR,bgG,bgB); 
            }
        }
    }

    screen_x_off+=8;
}

void vesa_1024_768_draw_string(const uint8_t* s) {
    while(*s) {
        vesa_1024_768_draw_char(*s);
        s++; 
    }
}

void vesa_1024_768_draw_int(int x, uint8_t r, uint8_t g, uint8_t b) {
    if(x == 0) {
        vesa_1024_768_draw_char('0');
        return;
    }
    if(x < 0) {
        vesa_1024_768_draw_char('-');
    }
    char buffer[12];
    int i = 0;
    while (x > 0) {
        buffer[i++] = (x % 10) + '0';
        x /= 10;
    }

    while (i > 0) {
        vesa_1024_768_draw_char(buffer[--i]);
    }
}

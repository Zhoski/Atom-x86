#include "vga_640_480.h"
#include "../../kernel/port/io.h"
#include "../../kernel/device/device.h"
#include "../../kernel/services/services.h"

#define FONT8x16_IMPLEMENTATION
#include "../../font/font8x16.h"

#define VGA_640_480_MEMORY (U8*)0xA0000

#define INDEX_REGISTER             0x3CE
#define DATA_REGISTER              0x3CF

#define SEQUENCER_CONTROLLER       0x3C4

#define MAP_MASK_REGISTER           0x02
#define SET_RESET_REGISTER          0x00
#define ENABLE_SET_RESET_REGISTER   0x01

#define GRAPHICS_MODE               0x05
#define BIT_MASK                    0x08

#define VGA_640_480_WIDTH           640
#define VGA_640_480_HEIGHT          480

#define VESA_1024_768_WIDTH        1024
#define VESA_1024_768_HEIGHT        768

U32 screen_x_off = 0;
U32 screen_y_off = 0;

U8 terminal_fg_vbe = 15;   // Белый
U8 terminal_bg_vbe = 0;    // Черный

void vga_640_480_clear_screen(U8 color) {
    screen_x_off = 0;
    screen_y_off = 0;

    outb(SEQUENCER_CONTROLLER, MAP_MASK_REGISTER);
    outb(DATA_REGISTER, 0x0F);

    outb(INDEX_REGISTER, GRAPHICS_MODE);
    outb(DATA_REGISTER, 0x02);

    outb(INDEX_REGISTER, 0x08); 
    outb(DATA_REGISTER, 0xFF);

    service.memory->memset(VGA_640_480_MEMORY, color, 640*80);

    outb(INDEX_REGISTER, GRAPHICS_MODE);
    outb(DATA_REGISTER, 0x00); 
}

void vga_640_480_write_row(U32 x, U32 y, U8 b, U8 fg, U8 bg) {
    U32 offset = ((y << 6) + (y << 4)) + (x >> 3);
    
    volatile U8 *address = VGA_640_480_MEMORY + offset;

    outb(INDEX_REGISTER, GRAPHICS_MODE);
    outb(DATA_REGISTER, 0x00);

    outb(0x03C4, 0x02);
    outb(0x03C5, 0x0F);

    outb(INDEX_REGISTER, 0x00);
    outb(DATA_REGISTER, bg);

    outb(0x03CE, 0x01); 
    outb(0x03CF, 0x0F);

    outb(INDEX_REGISTER, 0x08);
    outb(DATA_REGISTER, ~b);

    volatile U8 dummy = *address;
    
    *address = 0xFF;

    outb(INDEX_REGISTER, 0x00);
    outb(DATA_REGISTER, fg);

    outb(INDEX_REGISTER, 0x08);
    outb(DATA_REGISTER, b);

    dummy = *address;

    *address = 0xFF;
}

void vga_640_480_putpixel(U32 x, U32 y, U8 color) { 
    U32 offset = ((y << 6) + (y << 4)) + (x >> 3);
    U8 bit_mask = 0x80 >> (x & 7);
    
    volatile U8 *address = VGA_640_480_MEMORY + offset;

    outb(INDEX_REGISTER, GRAPHICS_MODE);
    outb(DATA_REGISTER, 0x02);
    outb(INDEX_REGISTER, BIT_MASK);
    outb(DATA_REGISTER, bit_mask);

    U8 dummy = *address;
    
    *address = color;
}

void vga_640_480_scroll() {
    U32 offset = 80 + VGA_640_480_MEMORY;
    U32 new_offset = 0 + VGA_640_480_MEMORY;

    for(U32 i = 0; i < 480;i++) {
        service.memory->memcpy(offset, new_offset, 80);
        offset += 80;
        new_offset += 80;
    }

    service.memory->memset(offset, terminal_bg_vbe, 80);
}

void vga_640_480_draw_char(U8 c) {
    if(c == '\n' || screen_x_off == 640) {
        screen_x_off = 0;
        screen_y_off += 16;
        if(screen_y_off == 464) {
            for(U32 i = 0;i < 16;i++) {
                vga_640_480_scroll();
            }
            screen_y_off -= 16;
        }
        return;
    }
    for(U32 row = 0; row < 16;row++) {
        U8 row_byte = font8x16[c][row];
        vga_640_480_write_row(screen_x_off, screen_y_off + row, row_byte, terminal_fg_vbe, terminal_bg_vbe);
    }

    screen_x_off+=8;
}

void vga_640_480_draw_string(const U8* s) {
    while(*s) {
        vga_640_480_draw_char(*s);
        s++; 
    }
}

void vga_640_480_set_cursor_position(const U16 x, const U16 y) {
    screen_x_off = x << 3;
    screen_y_off = y << 4;
}

void vga_640_480_get_cursor_position(U16* x, U16* y)  {
    *x = screen_x_off >> 3;
    *y = screen_y_off >> 4;
}

void vga_640_480_fg_vga_set(U8 color) {
    terminal_fg_vbe = color;
}

void vga_640_480_bg_vga_set(U8 color) {
    terminal_bg_vbe = color;
}
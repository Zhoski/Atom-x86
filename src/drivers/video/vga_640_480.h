#ifndef __VGA_640_480__
#define __VGA_640_480__
#include <stdint.h>

void vga_640_480_clear_screen(uint8_t color);
void vga_640_480_putpixel(uint32_t x, uint32_t y, uint8_t color);
void vga_640_480_draw_char(uint8_t c);
void vga_640_480_draw_string(const uint8_t* s);
void terminal_fg_vbe_set(uint8_t color);
void terminal_bg_vbe_set(uint8_t color);

#endif
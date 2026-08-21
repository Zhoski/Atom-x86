#ifndef __VGA_640_480__
#define __VGA_640_480__
#include <lib/int.h>

void vga_640_480_clear_screen(U8 color);
void vga_640_480_putpixel(U32 x, U32 y, U8 color);
void vga_640_480_draw_char(U8 c);
void vga_640_480_draw_string(const U8* s);
void vga_640_480_fg_vga_set(U8 color);
void vga_640_480_bg_vga_set(U8 color);
void vga_640_480_set_cursor_position(const U16 x, const U16 y);
void vga_640_480_get_cursor_position(U16* x, U16* y);
#endif
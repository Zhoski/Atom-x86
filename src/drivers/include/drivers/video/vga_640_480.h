#ifndef __VGA_640_480__
#define __VGA_640_480__
#include <lib/int.h>
#include <stdarg.h>

void vga_640_480_screen_clear(U8 color);
void vga_640_480_putpixel(U32 x, U32 y, U8 color);
void vga_640_480_draw_char(U8 c);
void vga_640_480_draw_string(const U8* s);
void vga_640_480_draw_int(U32 x);
void vga_640_480_fg_set(U8 color);
void vga_640_480_bg_set(U8 color);
void vga_640_480_cursor_set_position(const U16 x, const U16 y);
void vga_640_480_cursor_get_position(U16* x, U16* y);
void vga_640_480_kprintf(const u8 *format, ...);

#endif
#ifndef __VGA_80_25__
#define __VGA_80_25__
#include <lib/int.h>
#include <stdarg.h>

void vga_80_25_write_char(const u8 c);
void vga_80_25_write_string(const u8* s);
void vga_80_25_clear_screen(u8 color);

#endif
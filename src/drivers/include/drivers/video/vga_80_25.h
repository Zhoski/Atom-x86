#ifndef __VGA_80_25__
#define __VGA_80_25__
#include <lib/int.h>

void vga_80_25_write_char(const U8 c, U8 color);
void vga_80_25_write_string(const char* s, U8 color);
void vga_80_25_clear_screen(U8 color);

#endif
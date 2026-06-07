#ifndef __VGA_80_25__
#define __VGA_80_25__
#include <stdint.h>

void vga_80_25_write_char(const uint8_t c, uint8_t color);
void vga_80_25_write_string(const char* s, uint8_t color);
void vga_80_25_clear_screen(uint8_t color);

#endif
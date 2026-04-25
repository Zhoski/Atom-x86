#ifndef __VGA__
#define __VGA__
#include <stdint.h>

#define VGA_MEMORY 0xB8000
#define VGA_HEIGHT 25
#define VGA_WIDTH  80

void clear_screen();
void putchar(const char data);
void kwrite_string(const char* data);

#endif

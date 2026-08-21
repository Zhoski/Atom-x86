#ifndef VGA_H
#define VGA_H
#include <lib/int.h>

enum vga_color {
    VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

enum graphics_mode {
    VGA_80_25 = 0x03,          // VGA текстовый 80x25 16 цветов
    VGA_640_480 = 0x12,        // VGA графический 640x480 16 цветов
    VGA_320_200 = 0x13,        // VGA графический 320x200 256 цветов
};

typedef struct {
    void(*write_string)(U8* s);
    void(*write_char)(U8 c);
    void(*clear_screen)(U8 color);
	void(*terminal_fg_vbe_set) (U8 color);
	void(*terminal_bg_vbe_set) (U8 color);
	void (*terminal_set_cursor_position)(const U16 x, const U16 y);
	void (*terminal_get_cursor_position)(U16* x, U16* y)
}VideoDriver;

extern VideoDriver* video;

#endif

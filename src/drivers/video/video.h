#ifndef __VGA__
#define __VGA__
#include <stdint.h>

#define VGA_MEMORY 0xB8000
#define VGA_640_480_MEMORY (uint8_t*)0xA0000
#define VGA_HEIGHT 25
#define VGA_WIDTH  80

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

typedef struct {
    void(*write_string)(uint8_t* s);
    void(*write_char)(uint8_t c);
    void(*clear_screen)(uint8_t color);
    void(*set_screen_pos)(uint32_t x, uint32_t y);

    /* Только для графический режимов */
	void(*terminal_fg_vbe_set) (uint8_t color);
	void(*terminal_bg_vbe_set) (uint8_t color);
}Video;

extern Video video;

void clear_screen(uint8_t r, uint8_t g, uint8_t b);
void vga_80_25_write_char(const uint8_t c, uint8_t color, uint8_t n, uint8_t n2);
uint8_t vga_entry_color(uint8_t bg, uint8_t fg);
void vga_set_attribute(uint8_t bg, uint8_t fg);
void vga_80_25_write_string(const char* s, uint8_t color, uint8_t n, uint8_t n2);
//void vga_80_25_write_int(int x);
//void kwrite_hex(int x, int t);
void updateCursorPosition(uint8_t x, uint8_t y);
void init_vga(uint8_t graphics_mode);
// VGA
void vga_640_480_putpixel(uint32_t x, uint32_t y, uint8_t color);
void vga_640_480_draw_char(uint8_t c);
void vga_640_480_draw_string(const uint8_t* s);
void vga_640_480_clear_screen(uint8_t color);
void terminal_fg_vbe_set(uint8_t color);
void terminal_bg_vbe_set(uint8_t color);

#endif

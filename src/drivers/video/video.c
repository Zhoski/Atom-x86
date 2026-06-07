#include "video.h"
#include "vga_640_480.h"
#include "vga_80_25.h"

static VideoDriver vga_640_480 = {
    .write_string = &vga_640_480_draw_string,
    .write_char = &vga_640_480_draw_char,
    .clear_screen = &vga_640_480_clear_screen,
    .terminal_bg_vbe_set = &terminal_bg_vbe_set,
    .terminal_fg_vbe_set = &terminal_fg_vbe_set,
};

static VideoDriver vga_80_25 = {
    .write_string = &vga_80_25_write_string,
    .write_char = &vga_80_25_write_char,
    .clear_screen = &vga_80_25_clear_screen,
};

VideoDriver* video;

void init_vga(uint8_t mode) {
    switch (mode)
    {
        case VGA_640_480: video = &vga_640_480; break;
        case VGA_80_25  : video = &vga_80_25;   break;
        default: break;
    }
};
#include <drivers/video/video.h>
#include <drivers/video/vga_640_480.h>
#include <drivers/video/vga_80_25.h>

static VideoDriver vga_640_480 = {
    .write_string = &vga_640_480_draw_string,
    .write_char = &vga_640_480_draw_char,
    .clear_screen = &vga_640_480_clear_screen,
    .terminal_bg_vbe_set = &vga_640_480_bg_vga_set,
    .terminal_fg_vbe_set = &vga_640_480_fg_vga_set,
    .terminal_set_cursor_position = &vga_640_480_set_cursor_position,
    .terminal_get_cursor_position = &vga_640_480_get_cursor_position,
};

static VideoDriver vga_80_25 = {
    .write_string = &vga_80_25_write_string,
    .write_char = &vga_80_25_write_char,
    .clear_screen = &vga_80_25_clear_screen,
};

VideoDriver* video;

void init_vga(U8 mode) {
    switch (mode)
    {
        case VGA_640_480: video = &vga_640_480; break;
        case VGA_80_25  : video = &vga_80_25;   break;
        default: break;
    }
};
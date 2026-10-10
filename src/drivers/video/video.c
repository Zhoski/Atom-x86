#include <drivers/video/video.h>
#include <drivers/video/vga_640_480.h>

static VideoDriver_t vga_640_480 = {
    //.write_string = &vga_640_480_draw_string,
    .kputc = &vga_640_480_draw_char,
    .clear_screen = &vga_640_480_screen_clear,
    .terminal_bg_vbe_set = &vga_640_480_bg_set,
    .terminal_fg_vbe_set = &vga_640_480_fg_set,
    .terminal_set_cursor_position = &vga_640_480_cursor_set_position,
    .terminal_get_cursor_position = &vga_640_480_cursor_get_position,
    .write_int = &vga_640_480_draw_int,
    .kprintf = &vga_640_480_kprintf,
};

VideoDriver_t* video;

void init_vga(U8 mode) {
    switch (mode)
    {
        case VGA_640_480: video = &vga_640_480; break;
        default: break;
    }
};
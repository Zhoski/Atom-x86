#include <drivers/video/vga_640_480.h>
#include <cpu/io.h>
#include <kernel/device.h>
#include <kernel/services.h>

#define FONT8x16_IMPLEMENTATION
#include "../../font/font8x16.h"

#define VGA_640_480_MEMORY (u8*)0xA0000

#define INDEX_REGISTER             0x3CE
#define DATA_REGISTER              0x3CF

#define SEQUENCER_CONTROLLER       0x3C4

#define MAP_MASK_REGISTER           0x02
#define SET_RESET_REGISTER          0x00
#define ENABLE_SET_RESET_REGISTER   0x01

#define GRAPHICS_MODE               0x05
#define BIT_MASK                    0x08

#define VGA_640_480_WIDTH            640
#define VGA_640_480_HEIGHT           480

#define VESA_1024_768_WIDTH         1024
#define VESA_1024_768_HEIGHT         768

u32 screen_x_off = 0;
u32 screen_y_off = 0;

u8 terminal_fg_vbe = 15;
u8 terminal_bg_vbe = 0;

void vga_640_480_fg_opcode(u32 code) {
    vga_640_480_fg_set(code);
}

void vga_640_480_bg_opcode(u32 code) {
    vga_640_480_bg_set(code);
}

/**
 *  Очищает экран в цвет color
 */
void vga_640_480_screen_clear(u8 color) {
    screen_x_off = 0;
    screen_y_off = 0;

    outb(SEQUENCER_CONTROLLER, MAP_MASK_REGISTER);
    outb(DATA_REGISTER, 0x0F);

    outb(INDEX_REGISTER, GRAPHICS_MODE);
    outb(DATA_REGISTER, 0x02);

    outb(INDEX_REGISTER, 0x08); 
    outb(DATA_REGISTER, 0xFF);

    service.memory->memset(VGA_640_480_MEMORY, color, 640*80);

    outb(INDEX_REGISTER, GRAPHICS_MODE);
    outb(DATA_REGISTER, 0x00); 
}

void vga_640_480_write_row(u32 x, u32 y, u8 b, u8 fg, u8 bg) {
    u32 offset = ((y << 6) + (y << 4)) + (x >> 3);
    
    volatile u8 *address = VGA_640_480_MEMORY + offset;

    outb(INDEX_REGISTER, GRAPHICS_MODE);
    outb(DATA_REGISTER, 0x00);

    outb(0x03C4, 0x02);
    outb(0x03C5, 0x0F);

    outb(INDEX_REGISTER, 0x00);
    outb(DATA_REGISTER, bg);

    outb(0x03CE, 0x01); 
    outb(0x03CF, 0x0F);

    outb(INDEX_REGISTER, 0x08);
    outb(DATA_REGISTER, ~b);

    volatile u8 dummy = *address;
    
    *address = 0xFF;

    outb(INDEX_REGISTER, 0x00);
    outb(DATA_REGISTER, fg);

    outb(INDEX_REGISTER, 0x08);
    outb(DATA_REGISTER, b);

    dummy = *address;

    *address = 0xFF;
}

/**
 *  Закрашивает пиксель (x, y) в цвет color
 */
void vga_640_480_putpixel(u32 x, u32 y, u8 color) { 
    u32 offset = ((y << 6) + (y << 4)) + (x >> 3);
    u8 bit_mask = 0x80 >> (x & 7);
    
    volatile u8 *address = VGA_640_480_MEMORY + offset;

    outb(INDEX_REGISTER, GRAPHICS_MODE);
    outb(DATA_REGISTER, 0x02);
    outb(INDEX_REGISTER, BIT_MASK);
    outb(DATA_REGISTER, bit_mask);

    u8 dummy = *address;
    
    *address = color;
}

/**
 *  Прокручивает экран вниз на один символ
 */
void vga_640_480_scroll() {
    u32 offset = 80 + VGA_640_480_MEMORY;
    u32 new_offset = 0 + VGA_640_480_MEMORY;

    for(u32 i = 0; i < 480;i++) {
        service.memory->memcpy(offset, new_offset, 80);
        offset += 80;
        new_offset += 80;
    }

    service.memory->memset(offset, terminal_bg_vbe, 80);
}

/**
 *  Выводит символ, прокручивает экран
 */
void vga_640_480_draw_char(u8 c) {
    if(c == '\n' || screen_x_off == 640) {
        screen_x_off = 0;
        screen_y_off += 16;
        if(screen_y_off == 464) {
            for(u32 i = 0;i < 16;i++) {
                vga_640_480_scroll();
            }
            screen_y_off -= 16;
        }
        return;
    }
    for(u32 row = 0; row < 16;row++) {
        u8 row_byte = font8x16[c][row];
        vga_640_480_write_row(screen_x_off, screen_y_off + row, row_byte, terminal_fg_vbe, terminal_bg_vbe);
    }

    screen_x_off+=8;
}

void vga_640_480_draw_string(const u8* s) {
    while(*s) {
        vga_640_480_draw_char(*s);
        s++; 
    }
}

void vga_640_480_draw_int(u32 x) {
    u32 i = 0;
    u32 isNegative = 0;

    u8 buffer[10];

    if (x < 0) {
        isNegative = 1;
        x = -x;
    }

    do {
        buffer[i++] = (x % 10) + '0';
        x /= 10;
    } while (x > 0);

    if (isNegative) {
        buffer[i++] = '-';
    }

    buffer[i] = '\0';

    u32 start = 0;
    u32 end = i - 1;
    while (start < end) {
        u8 temp = buffer[start];
        buffer[start] = buffer[end];
        buffer[end] = temp;
        start++;
        end--;
    }

    vga_640_480_draw_string(buffer);
}

void vga_640_480_cursor_set_position(const u16 x, const u16 y) {
    screen_x_off = x << 3;
    screen_y_off = y << 4;
}

void vga_640_480_cursor_get_position(u16* x, u16* y)  {
    *x = screen_x_off >> 3;
    *y = screen_y_off >> 4;
}

void vga_640_480_fg_set(u8 color) {
    terminal_fg_vbe = color;
}

void vga_640_480_bg_set(u8 color) {
    terminal_bg_vbe = color;
}

/**
 *  Форматированный вывод бесконечного числа аргументов
 */
void vga_640_480_kprintf(const u8 *format, ...) {
    va_list args;
    va_start(args, format);

    for(const i8 *p = format; *p != '\0'; p++) {
        if(*p != '%') {
            vga_640_480_draw_char(*p);
            continue;
        }

        p++;

        switch (*p)
        {
        case 'd':
            i32 n = va_arg(args, i32);
            i32 i = 0;
            i32 isNegative = 0;

            i8 buffer[10] = {0};

            if (n < 0) {
                isNegative = 1;
                n = -n;
            }

            do {
                buffer[i++] = (n % 10) + '0';
                n /= 10;
            } while (n > 0);

            if (isNegative) {
                buffer[i++] = '-';
            }

            buffer[i] = '\0';

            i32 start = 0;
            i32 end = i - 1;
            while (start < end) {
                i8 temp = buffer[start];
                buffer[start] = buffer[end];
                buffer[end] = temp;
                start++;
                end--;
            }
            
            vga_640_480_draw_string(buffer);

            break;
        case 's':
            i8 *s = va_arg(args, i8*);
            vga_640_480_draw_string(s);
            break;
        case 'f': {
            const i8 code[2] = {*(p+1), *(p+2)};
            u32 result = 0;
            i8 c = code[0];

            result = result * 10 + (c - '0');

            c = code[1];

            result = result * 10 + (c - '0');

            vga_640_480_fg_opcode(result);

            p += 2;

            break;
        }
        case 'b': {
            const i8 code[2] = {*(p+1), *(p+2)};
            u32 result = 0;
            i8 c = code[0];

            result = result * 10 + (c - '0');

            c = code[1];

            result = result * 10 + (c - '0');

            vga_640_480_bg_opcode(result);

            p += 2;

            break;
        }
        default:
            break;
        }
    }
}
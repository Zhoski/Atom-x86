#include "libio.h"
#include <stdarg.h>

void sequence_black() {
    SetFGColor(0,0,0);
}

void sequence_blue() {
    SetFGColor(0,0,255);
}

void sequence_green() {
    SetFGColor(0,255,0);
}


void sequence_cyan() {
    SetFGColor(0,255,255);
}

void sequence_red() {
    SetFGColor(255,0,0);
}

void sequence_magenta() {
    SetFGColor(255,0,255);
}

void sequence_brown() {
    SetFGColor(150,75,0);
}

void sequence_light_grey() {
    SetFGColor(170, 170, 170);
}

void sequence_dark_grey() {
    SetFGColor(85, 85, 85);
}

void sequence_light_blue() {
    SetFGColor(85, 85, 255);
}

void sequence_light_green() {
    SetFGColor(85, 255, 85);
}

void sequence_light_cyan() {
    SetFGColor(85, 255, 255);
}

void sequence_light_red() {
    SetFGColor(255, 85, 85);
}

void sequence_light_magenta() {
    SetFGColor(255, 85, 255);
}

void sequence_light_brown() {
    SetFGColor(255, 255, 85); // В VGA это ярко-желтый (Yellow)
}

void sequence_white() {
    SetFGColor(255, 255, 255);
}

typedef struct {
    uint8* sequence;
    void (*handler)();
}control_sequence;

const control_sequence vga_colors[] = {
    {.sequence = "$[00", .handler = sequence_black},
    {.sequence = "$[01", .handler = sequence_blue},
    {.sequence = "$[02", .handler = sequence_green},
    {.sequence = "$[03", .handler = sequence_cyan},
    {.sequence = "$[04", .handler = sequence_red},
    {.sequence = "$[05", .handler = sequence_magenta},
    {.sequence = "$[06", .handler = sequence_brown},
    {.sequence = "$[07", .handler = sequence_light_grey},
    {.sequence = "$[08", .handler = sequence_dark_grey},
    {.sequence = "$[09", .handler = sequence_light_blue},
    {.sequence = "$[10", .handler = sequence_light_green},
    {.sequence = "$[11", .handler = sequence_light_cyan},
    {.sequence = "$[12", .handler = sequence_light_red},
    {.sequence = "$[13", .handler = sequence_light_magenta},
    {.sequence = "$[14", .handler = sequence_light_brown},
    {.sequence = "$[15", .handler = sequence_white}
};

int __strcmp(const char *s1, const char *s2) {
    while(*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

void SetFGColor(unsigned char r,unsigned char g,unsigned char b) {
    uint32 color = 0x00000000;

    uint32 _r = r;
    _r <<= 24;
    uint32 _g = g;
    _g <<= 16;
    uint32 _b = b;
    _b <<= 8;

    color |= _r;
    color |= _g;
    color |= _b;
    asm volatile(
        "movl $3, %%eax\n"
        "movl $1, %%ebx\n"
        "movl %0, %%ecx\n"
        "int $0x80"
        :
        : "r"(color)
        : "eax", "ebx", "ecx", "memory"
    );
}

void SetBGColor(unsigned char r,unsigned char g,unsigned char b) {
    uint32 color = 0x00000000;

    uint32 _r = r;
    _r <<= 24;
    uint32 _g = g;
    _g <<= 16;
    uint32 _b = b;
    _b <<= 8;

    color |= _r;
    color |= _g;
    color |= _b;
    asm volatile(
        "movl $3, %%eax\n"
        "movl $2, %%ebx\n"
        "movl %0, %%ecx\n"
        "int $0x80"
        :
        : "r"(color)
        : "eax", "ebx", "ecx", "memory"
    );
}

void printf(unsigned char s[]) {
    uint8 buffer[5] = {0};
    while (*s)
    {
        for(uint32 i = 0;i < 4;i++) {
            buffer[i] = *(s + i);
        }

        for(uint32 i = 0;i < 16;i++) {
            if(__strcmp(buffer, vga_colors[i].sequence) == 0) {
                vga_colors[i].handler();
                s += 4;
                continue;
            }
        }

        /*if(__strcmp(buffer, "$[04") == 0) {
            SetFGColor(255,0,0);
            s += 4;
            continue;
        }*/

        asm volatile(
            "movl $1, %%eax\n"
            "movl $2, %%ebx\n"
            "movl %0, %%ecx\n"
            "int $0x80"
            :
            : "r" ((uint32)*s)
            : "eax", "ebx", "ecx", "memory"
        );
        s++;
    }
    
}

void putchar(unsigned char c) {
    asm volatile (
        "movl $1, %%eax\n"
        "movl $2, %%ebx\n"
        "movl %0, %%ecx\n"
        "int $0x80"
        :
        : "r" ((unsigned int)c)
        : "eax", "ebx","ecx", "memory"
    );
}

void clear_screen(unsigned char r, unsigned char g, unsigned char b) {
    uint32 color = 0x00000000;

    uint32 _r = r;
    _r <<= 24;
    uint32 _g = g;
    _g <<= 16;
    uint32 _b = b;
    _b <<= 8;

    color |= _r;
    color |= _g;
    color |= _b;

    asm volatile (
        "movl $3, %%eax\n"
        "movl $3, %%ebx\n"
        "movl %0, %%ecx\n"
        "int $0x80"
        :
        : "r" (color)
        : "eax", "ebx", "ecx", "memory"
    );
}

uint8 get_char() {
    uint32 out; 
    asm volatile (
        "movl $2, %%eax\n"
        "movl $1, %%ebx\n"
        "movl $0, %%ecx\n"
        "movl $0, %%edx\n"
        "int $0x80"
        : "=a"(out) 
        : 
        : "ebx", "ecx", "edx", "memory"
    );
    return (uint8)out;
}

#include "strio.h"
#include <stdarg.h>

void sequence_black() {
    SetFGColor(0);
}

void sequence_blue() {
    SetFGColor(1);
}

void sequence_green() {
    SetFGColor(2);
}


void sequence_cyan() {
    SetFGColor(3);
}

void sequence_red() {
    SetFGColor(4);
}

void sequence_magenta() {
    SetFGColor(5);
}

void sequence_brown() {
    SetFGColor(6);
}

void sequence_light_grey() {
    SetFGColor(7);
}

void sequence_dark_grey() {
    SetFGColor(8);
}

void sequence_light_blue() {
    SetFGColor(9);
}

void sequence_light_green() {
    SetFGColor(10);
}

void sequence_light_cyan() {
    SetFGColor(11);
}

void sequence_light_red() {
    SetFGColor(12);
}

void sequence_light_magenta() {
    SetFGColor(13);
}

void sequence_light_brown() {
    SetFGColor(14);
}

void sequence_white() {
    SetFGColor(15);
}

typedef struct {
    uint8* sequence;
    void (*handler)();
}control_sequence;

const control_sequence vga_colors[] = {
    {.sequence = "00", .handler = sequence_black},
    {.sequence = "01", .handler = sequence_blue},
    {.sequence = "02", .handler = sequence_green},
    {.sequence = "03", .handler = sequence_cyan},
    {.sequence = "04", .handler = sequence_red},
    {.sequence = "05", .handler = sequence_magenta},
    {.sequence = "06", .handler = sequence_brown},
    {.sequence = "07", .handler = sequence_light_grey},
    {.sequence = "08", .handler = sequence_dark_grey},
    {.sequence = "09", .handler = sequence_light_blue},
    {.sequence = "10", .handler = sequence_light_green},
    {.sequence = "11", .handler = sequence_light_cyan},
    {.sequence = "12", .handler = sequence_light_red},
    {.sequence = "13", .handler = sequence_light_magenta},
    {.sequence = "14", .handler = sequence_light_brown},
    {.sequence = "15", .handler = sequence_white}
};

int __strcmp(const char *s1, const char *s2) {
    while(*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

void SetFGColor(unsigned char color) {
    asm volatile(
        "movl $3, %%eax\n"
        "movl $1, %%ebx\n"
        "movl %0, %%ecx\n"
        "int $0x80"
        :
        : "r"((uint32)color)
        : "eax", "ebx", "ecx", "memory"
    );
}

void SetBGColor(unsigned char color) {
    asm volatile(
        "movl $3, %%eax\n"
        "movl $2, %%ebx\n"
        "movl %0, %%ecx\n"
        "int $0x80"
        :
        : "r"((uint32)color)
        : "eax", "ebx", "ecx", "memory"
    );
}

void printf(const char *format, ...) {
    va_list args;
    va_start(args, format);

    for(const char *p = format; *p != '\0'; p++) {
        if(*p != '%') {
            asm volatile (
                "movl $1, %%eax\n"
                "movl $2, %%ebx\n"
                "movl %0, %%ecx\n"
                "int $0x80"
                :
                : "r" ((unsigned int)*p)
                : "eax", "ebx","ecx", "memory"
            );
            continue;
        }

        p++;

        switch (*p)
        {
        case 'd':
            int n = va_arg(args, int);
            int i = 0;
            int isNegative = 0;

            char *buffer;

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

            int start = 0;
            int end = i - 1;
            while (start < end) {
                char temp = buffer[start];
                buffer[start] = buffer[end];
                buffer[end] = temp;
                start++;
                end--;
            }
            
            asm volatile (
                "movl $1, %%eax\n"
                "movl $1, %%ebx\n"
                "movl %0, %%ecx\n"
                "int $0x80"
                :
                : "r" (buffer)
                : "eax", "ebx","ecx", "memory"
            );

            break;
        case 's':
            char *s = va_arg(args, char*);
            asm volatile (
                "movl $1, %%eax\n"
                "movl $1, %%ebx\n"
                "movl %0, %%ecx\n"
                "int $0x80"
                :
                : "r" (s)
                : "eax", "ebx","ecx", "memory"
            );
            break;
        case '[':
            const char code[3] = {*(p+1), *(p+2), 0};  // Следущие два символа
            for(int i = 0;i < 16;i++) {
                if(__strcmp(code, vga_colors[i].sequence) == 0) {
                    vga_colors[i].handler();
                    break;
                }
            }
            p += 2;
            break;
        default:
            break;
        }
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

void clear_screen(unsigned char color) {
    asm volatile (
        "movl $3, %%eax\n"
        "movl $3, %%ebx\n"
        "movl %0, %%edx\n"
        "int $0x80"
        :
        : "r" ((uint32)color)
        : "eax", "ebx", "edx", "memory"
    );
}

void set_cursor(const unsigned short x, const unsigned short y) {
    uint32 pos = x;
    pos <<= 16;
    pos |= y;
    
    asm volatile (
        "movl $3, %%eax\n"
        "movl $4, %%ebx\n"
        "movl %0, %%ecx\n"
        "int $0x80"
        :
        : "r" (pos)
        : "eax", "ebx", "ecx", "memory"
    );
}

void get_cursor(unsigned int* x, unsigned int* y) { 
    asm volatile (
        "movl $3, %%eax\n"
        "movl $5, %%ebx\n"
        "movl %0, %%ecx\n"
        "movl %1, %%edx\n"
        "int $0x80"
        :
        : "r" (x), "r" (y)
        : "eax", "ebx", "ecx", "edx", "memory"
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

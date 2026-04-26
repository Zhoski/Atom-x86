#include "vga.h"

static inline void outb(uint16_t port, uint8_t data) {
    asm volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t data;
    asm volatile("inb %1, %0" : "=a"(data) : "Nd"(port));
    return data;
}

void keyboard_handler() {
    uint8_t scancode = inb(0x60);
    if(scancode & 0x80) {
        return;
    }

    putchar('!');
    
}

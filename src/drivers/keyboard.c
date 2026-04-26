#include "vga.h"
#include "keyboard.h"

#define KEYBOARD_BUF_SIZE   32

uint8_t _keyboard_buf[KEYBOARD_BUF_SIZE] = {0};
uint8_t _keyboard_buf_insert = 1;
uint8_t _keyboard_buf_read   = 0;

static inline void outb(uint16_t port, uint8_t data) {
    asm volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t data;
    asm volatile("inb %1, %0" : "=a"(data) : "Nd"(port));
    return data;
}

void keyboard_buf_insert(uint8_t c) {
    if(_keyboard_buf_insert < KEYBOARD_BUF_SIZE) {
        _keyboard_buf[_keyboard_buf_insert++] = c;
        _keyboard_buf_read = _keyboard_buf_insert - 1;
    }else {
        _keyboard_buf_insert = 0;
        _keyboard_buf_read = 0;
        _keyboard_buf[_keyboard_buf_insert++] = c;
    }
} 

uint8_t keyboard_buf_get_las_sym() {
    uint8_t temp = _keyboard_buf[_keyboard_buf_read];
    return temp;
}

void keyboard_handler() {
    uint8_t scancode = inb(0x60);
    if(scancode & 0x80) {
        return;
    }
    uint8_t c = ascii_table[scancode];
    keyboard_buf_insert(c);
    if(c != '\n' && c != 0 && c != '\t' && c != '\b') {
        putchar(c);
    } 
    
}

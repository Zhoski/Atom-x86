#include "vga.h"
#include "keyboard.h"

#define KEYBOARD_BUF_SIZE   32

const unsigned char ascii_table[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', /* 0x00 - 0x09 */
    '9', '0', '-', '=', '\b', '\t',                 /* 0x0A - 0x0F */
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', /* 0x10 - 0x19 */
    '[', ']', '\n', 0, 'a', 's', 'd', 'f', 'g', 'h',  /* 0x1A - 0x23 */
    'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', /* 0x24 - 0x2D */
    'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*',  /* 0x2E - 0x37 */
    0, ' ', 0, 0, 0, 0, 0, 0, 0, 0,                   /* 0x38 - 0x41 */
    0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4',     /* 0x42 - 0x4D */
    '5', '6', '+', '1', '2', '3', '0', '.', 0, 0, 0,  /* 0x4E - 0x58 */
    0, 0 /* Остальные — нули */
};

uint8_t _keyboard_buf[KEYBOARD_BUF_SIZE];
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
    if(_keyboard_buf[_keyboard_buf_read]!=0) {
        uint8_t temp = _keyboard_buf[_keyboard_buf_read];
        _keyboard_buf[_keyboard_buf_read] = 0;
        return temp;
    } 
    return 0; 
}

void keyboard_handler() {
    uint8_t scancode = inb(0x60);
    if(scancode & 0x80) {
        return;
    }
    uint8_t c = ascii_table[scancode];
    keyboard_buf_insert(c);
    //if(c != '\n' && c != 0 && c != '\t' && c != '\b') {
    //    putchar(c);
    //} 
    
}

#include "../VGA/vga.h"
#include "keyboard.h"

#define KEYBOARD_BUF_SIZE   32

#define SHIFT 0x01
#define CAPS  0x02
#define CTRL  0x03
#define ENTER 0x0A // '\n'
#define BACKSPACE 0x08 // '\b'

const unsigned char ascii_table[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', 
    '9', '0', '-', '=', '\b', '\t',                 
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', 
    '[', ']', '\n', 0x03, 'a', 's', 'd', 'f', 'g', 'h', 
    'j', 'k', 'l', ';', '\'', '`', 0x01, '\\', 'z', 'x',
    'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0x01, '*',
    0, ' ', 0x02, 0, 0, 0, 0, 0, 0, 0,                 
    0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4',     
    '5', '6', '+', '1', '2', '3', '0', '.', 127, 0, 0, 
    0, 0 
};
uint8_t _keyboard_buf[KEYBOARD_BUF_SIZE];
uint8_t _keyboard_buf_insert = 1;
uint8_t _keyboard_buf_read   = 0;
uint8_t isShift = 0;
uint8_t isCaps = 0;

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
    if(c  == SHIFT) { 
        isShift = 1;
        return;
    }else if (c == CAPS) {
        isCaps = !isCaps;
    }else {
        if(isCaps) {            
            keyboard_buf_insert(c-32); 
        } 
        else {
            keyboard_buf_insert(c); 
        }
    }  
}

#include "../../kernel/port/io.h"
#include "../../kernel/device/device.h"
#include "keyboard.h"

#define KEYBOARD_BUF_SIZE       32

// Порты
#define KEYBOARD_DATA_PORT      0x60
#define KEYBOARD_COMMAND_PORT   0x64

#define KEY_RELEASE_BIT         0x80

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

static keyboard_interface kb_ops = {
    .get_last_key = keyboard_buf_get_las_sym
};

device kb_device; 

void init_keyboard() {
    kb_device.name = "kb";
    kb_device.key  = &kb_ops; 
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
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    if(scancode & KEY_RELEASE_BIT ) {   
        return;
    } 
    uint8_t c = ascii_table[scancode];
    keyboard_buf_insert(c); 
}

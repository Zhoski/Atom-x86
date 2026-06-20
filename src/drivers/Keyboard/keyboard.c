#include "../../kernel/port/io.h"
#include "../../kernel/device/device.h"
#include "keyboard.h"
#include "../video/video.h"

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

const U8 ascii_table[128] __attribute__((aligned(4))) = {
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
U8 _keyboard_buf[KEYBOARD_BUF_SIZE];
U8 _keyboard_buf_insert = 1;
U8 _keyboard_buf_read   = 0;
U8 isShift = 0;
U8 isCaps = 0;

keyboard_interface kb; 

void init_keyboard() {
    kb.get_last_key = &keyboard_buf_get_las_sym; 
}

void keyboard_buf_insert(U8 c) {
    if(_keyboard_buf_insert < KEYBOARD_BUF_SIZE) {
        _keyboard_buf[_keyboard_buf_insert++] = c;
        _keyboard_buf_read = _keyboard_buf_insert - 1;
    }else {
        _keyboard_buf_insert = 0;
        _keyboard_buf_read = 0;
        _keyboard_buf[_keyboard_buf_insert++] = c;
    }
} 

U8 keyboard_buf_get_las_sym() {
    if(_keyboard_buf[_keyboard_buf_read]!=0) {
        U8 temp = _keyboard_buf[_keyboard_buf_read];
        _keyboard_buf[_keyboard_buf_read] = 0;
        return temp;
    } 
    return 0; 
}

void keyboard_handler() {
    U8 scancode = inb(KEYBOARD_DATA_PORT);
    if(scancode & KEY_RELEASE_BIT ) {   
        return;
    } 
    U8 c = ascii_table[scancode];
    keyboard_buf_insert(c); 
}

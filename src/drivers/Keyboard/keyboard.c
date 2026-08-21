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
    0,   27,  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  // 0-9   (0x1B = Esc)
    '9', '0', '-',  '=',  '\b', '\t', 'q',  'w',  'e',  'r',  // 10-19 (\b = Backspace, \t = Tab)
    't', 'y', 'u',  'i',  'o',  'p',  '[',  ']',  '\n', 0x03, // 20-29 (\n = Enter, 0x03 = Левый Ctrl)
    'a', 's', 'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',  // 30-39
    '\'', '`', 0x01, '\\', 'z',  'x',  'c',  'v',  'b',  'n',  // 40-49 (0x01 = Левый Shift)
    'm', ',', '.',  '/',  0x01, '*',  0,    ' ',  0x02, 0x11, // 50-59 (0x01 = Пр Shift, 0x02 = Caps, 0x11 = F1)
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0,  // 60-69 (0x12=F2, 0x13=F3... 0x1A=F10)
    0,   '7', '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',  // 70-79 (Цифровая клавиатура)
    '2', '3', '0',  '.',  127,  0,    0,    0x1B, 0x1C, 0,    // 80-89 (127 = Delete, 0x1B = F11, 0x1C = F12)
    0,   0,   0,    0,    0,    0,    0,    0,    0,    0,    // 90-99
    0,   0,   0,    0,    0,    0,    0,    0,    0,    0,    // 100-109
    0x04, 0,  0x05, 0,    0x06, 0,    0x07, 0,    0,    0,    // 110-119 (0x04=Вверх, 0x05=Вниз, 0x06=Влево, 0x07=Вправо)
    0,   0,   0,    0,    0,    0,    0,    0                 // 120-127
};

const U8 ascii_table_shift[128] __attribute__((aligned(4))) = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', 
    '(', ')', '_', '+', '\b', '\t',                 
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', 
    '{', '}', '\n', 0x03, 'A', 'S', 'D', 'F', 'G', 'H', 
    'J', 'K', 'L', ':', '"', '~', 0x01, '|', 'Z', 'X',
    'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0x01, '*',
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
        if(ascii_table[scancode] == SHIFT) {
            isShift = 0;
        }
        return;
    } 
    U8 c = ascii_table[scancode];
    if(c == SHIFT) {
        isShift = 1;
    }
    if(isShift) {
        c = ascii_table_shift[scancode];
    }
    keyboard_buf_insert(c); 
}

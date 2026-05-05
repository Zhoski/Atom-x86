#ifndef __KEYBOARD__
#define __KEYBOARD__
extern const unsigned char ascii_table[128];

extern uint8_t keyboard_buf_get_las_sym();
extern void init_keyboard();
#endif

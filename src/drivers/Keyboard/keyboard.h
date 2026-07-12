#ifndef __KEYBOARD__
#define __KEYBOARD__
#include "../../kernel/int.h"
extern const U8 ascii_table[128];
extern const U8 ascii_table_shift[128];

U8 keyboard_buf_get_las_sym();
void init_keyboard();
#endif

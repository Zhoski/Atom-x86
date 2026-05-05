#ifndef __DEVICE__
#define __DEVICE__
#include <stdint.h>

typedef struct {
    uint8_t (*get_last_key)();
} keyboard_interface;

//typedef struct {
//    void (*update_cursor_position)(uint8_t, uint8_t);
//    uint8_t (*vga_entry_color)(uint8_t,uint8_t);
//    void (*clear_screen)();
//    void (*vga_set_attribute)();
//   void (*putchar)(uint8_t);
//    void (*kwrite_string)(const char* data);
//    void (*kwrite_int)(int);
//    void (*kwrite_hex)(int, int);
//} vga_interface;

typedef struct {
    const char *name;
    keyboard_interface *key;
    //vga_interface      *vga;
} device;

extern device kb_device;
//extern device vga_device;


#endif

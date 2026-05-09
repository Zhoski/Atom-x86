#ifndef __DEVICE__
#define __DEVICE__
#include <stdint.h>

typedef struct {
    uint8_t (*get_last_key)();
} keyboard_interface;

typedef struct {
    uint8_t (*init_pata)();
    uint8_t (*read_sector)(uint32_t, uint16_t word[256]);
    uint8_t (*write_sector)(uint32_t, uint16_t word[256]);
} disk_interface;

typedef struct {
    const char *name;
    keyboard_interface *key;
    disk_interface     *disk;
} device;

extern device kb_device;
extern device disk_device;


#endif

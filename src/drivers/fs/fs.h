#ifndef FS
#define FS
#include <stdint.h>

typedef struct
{
    uint8_t (*open)(const uint8_t *file_name);
    uint8_t (*read)(const uint8_t *file_name, uint8_t* out);
    uint8_t (*create)(const uint8_t *file_name, uint16_t size);
    uint8_t (*delete)(const uint8_t *file_name);
} FileSystem;

void init_fs();

extern FileSystem* fs;

#endif
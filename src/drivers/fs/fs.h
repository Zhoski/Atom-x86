#ifndef FS
#define FS
#include <stdint.h>

typedef struct
{
    uint8_t (*open)(const uint8_t *file_name);
} FileSystem;

void init_fs();

extern FileSystem* fs;

#endif
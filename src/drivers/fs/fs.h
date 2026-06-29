#ifndef FS
#define FS
#include <stdint.h>

typedef struct
{
    uint8_t (*check)(const uint8_t *__restrict__ file_name);
    uint8_t (*open)(const uint8_t *__restrict__ file_name);
    uint8_t (*read)(const uint8_t *__restrict__ file_name, uint8_t *__restrict__ out);
    uint8_t (*create)(const uint8_t *__restrict__ file_name, uint16_t size);
    uint8_t (*delete)(const uint8_t *__restrict__ file_name);
    uint8_t (*update) (const uint8_t *__restrict__ file_name, uint8_t* in, uint32_t bytes);
    uint8_t (*get_root) (uint8_t *__restrict__ out);
    uint8_t (*init)();
} FileSystem;

void init_fs();

extern FileSystem* fs;

#endif
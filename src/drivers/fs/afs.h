#ifndef __AFS__
#define __AFS__
#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint8_t name[8];
    uint8_t ext[3];
    uint16_t start_sec;
    uint16_t size;
    uint8_t flags;
} File;

uint8_t find_file(const uint8_t *file_name);
uint8_t afs_open(const uint8_t *file_name);
uint8_t afs_read(const uint8_t *file_name, uint8_t buffer[512]);
uint8_t afs_create(const uint8_t* file_name, uint16_t size);
uint8_t afs_delete(const uint8_t* file_name);
uint8_t afs_update(const uint8_t* file_name, uint8_t* in, uint32_t bytes);
uint8_t afs_init();

#endif

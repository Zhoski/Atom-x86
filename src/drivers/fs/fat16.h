#ifndef FAT16
#define FAT16
#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint8_t name[8];
    uint8_t ext[3];
    uint16_t start_sec;
    uint16_t size_in_sec;
    uint8_t flags;
} File;

void init_fat16(uint16_t bootSector[256]);

uint8_t find_file(const uint8_t *file_name);
uint8_t open(const uint8_t *file_name);
uint8_t read_file(const uint8_t *file_name, uint8_t buffer[512]);

#endif

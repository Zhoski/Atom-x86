#ifndef FAT16
#define FAT16
#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint8_t name[8];
    uint8_t ext[3];
    uint8_t atr;
    uint8_t reserved[10];
    uint16_t t_creation;
    uint16_t d_creation;
    uint16_t first_clus;
    uint32_t size_in_b;
} File;

void init_fat16(uint16_t bootSector[256]);
uint8_t find_file(const uint8_t *file_name);
uint8_t open(const uint8_t *file_name) ;

#endif

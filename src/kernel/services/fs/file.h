#ifndef __FILE_H_
#define __FILE_H_
#include <stdint.h>

typedef struct {
    uint8_t name[16];
    uint32_t entry_sector;
    uint32_t size_in_sector;
} file;

#endif

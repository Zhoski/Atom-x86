#ifndef __FILE_SYSTEM__
#define __FILE_SYSTEM__
#define MAX_FILE            16
#define SIZE_ONE_RECORD     24  // byte
#define TABLE_BASE_ADRES    0x4000
#include "../Memory/memory.h"
#include <stdint.h>

typedef struct {
    uint8_t name[16];
    uint32_t sector_entry;
    uint32_t size_in_sector;
} File;

typedef File file;

extern void init_table();
extern void open();
extern void read();
extern void write();

#endif

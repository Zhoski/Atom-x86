#ifndef __PATA__
#define __PATA__

#include <stdint.h>

extern uint8_t init_pata(uint16_t info[256]);
extern void disk_handler();
extern uint8_t read_sector(uint32_t lba, uint16_t word[256]);
extern void write_sector(uint32_t lba, uint16_t word[256]);

#endif

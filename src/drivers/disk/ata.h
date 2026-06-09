#ifndef __ATA__
#define __ATA__

#include <stdint.h>

void ata_disk_handler();
uint8_t init_ata(uint16_t info[256]);
uint8_t ata_read_sector(uint32_t lba, uint16_t word[256]);
uint8_t ata_write_sector(uint32_t lba, uint16_t word[256]);

#endif

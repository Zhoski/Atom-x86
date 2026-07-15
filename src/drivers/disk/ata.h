#ifndef __ATA__
#define __ATA__

#include <stdint.h>
#include "../../kernel/int.h"

void ata_disk_handler();
U8 init_ata(U16 info[256]);
U8 ata_read_sector(U32 lba, U16 word[256]);
U8 ata_write_sector(U32 lba, U16 word[256]);

#endif

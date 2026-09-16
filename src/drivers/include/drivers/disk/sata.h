#ifndef SATA_H
#define SATA_H

#include <lib/int.h>

typedef struct HBA_regs {
    U32 cap;
    U32 ghc;
    U32  is;
    U32  pi;
} __attribute__((packed));

typedef struct sata_disk {
    
};

U32 init_sata(U16 info[256]);
U8 sata_read_sector(U32 lba, U16 word[256]);
U8 sata_write_sector(U32 lba, U16 word[256]);

U32 ahci_mem_base = 0;

#endif
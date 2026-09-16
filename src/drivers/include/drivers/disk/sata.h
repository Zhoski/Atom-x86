#ifndef SATA_H
#define SATA_H

#include <lib/int.h>

typedef struct HBA_regs {
    U32 cap;
    U32 ghc;
    U32  is;
    U32  pi;
} __attribute__((packed));

typedef struct HBA_port_regs {
    U32      clb;
    U32     clbu;
    U32       fb;
    U32      fbu;
    U32       is;
    U32       ie;
    U32      cmd;
    U32      rsv;
    U32      tfd;
    U32     ssts;
    U32     sctl;
    U32     serr;
    U32     sact;
    U32       ci;
    U32     sntf;
    U32      fbs;

    U8 rsv1[0x7F - 0x44];

} __attribute__((packed));

typedef struct sata_disk {
    
};

U32 init_sata(U16 info[256]);
U8 sata_read_sector(U32 lba, U16 word[256]);
U8 sata_write_sector(U32 lba, U16 word[256]);

U32 ahci_mem_base = 0;

#endif
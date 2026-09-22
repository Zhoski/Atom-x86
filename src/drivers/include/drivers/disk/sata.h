#ifndef SATA_H
#define SATA_H

#include <lib/int.h>

typedef struct HBA_port_mem;

struct HBA_port_mem {
    u32  clb;
    u32 clbu;
    u32   fb;
    u32  fbu;
    u32   is;
    u32   ie;
    u32  cmd;
    u32 rsv0;
    u32  tfd;
    u32  sig;
    u32 ssts;
    u32 sctl;
    u32 serr;
    u32 sact;
    u32   ci;
    u32 sntf;
    u32  fbs;
    u32 devslp;
    u8  rsv1[56];
} __attribute__((packed));

struct HBA_mem {
    u32 cap;
    u32 ghc;
    u32 is;
    u32 pi;
    u32 vs;
    u32 ccc_ctl;
    u32 ccc_pts;
    u32 em_loc;
    u32 em_ctl;
    u32 cap2;
    u32 bohc;

    u8  rsv[116];  

    u8  vendor[96]; 

    struct HBA_port_mem port[32]; 
} __attribute__((packed));

typedef struct HBA_cmd_header {
    u16 w0;
    u16 prdtl;
    u32 prdbc;
    u32 ctba;
    u32 ctbau;

    u8  rsv[16];

} __attribute__((packed));

typedef struct HBA_prdt_entry {
    u32  dba;
    u32 dbau;
    u32 rsv0;

    u32 dbc:22;     
    u32 rsv1:9; 
    u32 i:1;

} __attribute__((packed));

typedef struct HBA_cmd_table {
    u8       cfis[64];
    u8       acmd[16];
    u8        rsv[48];

    struct HBA_prdt_entry prdt_entry;
    
} __attribute__((packed));

typedef struct HBA_fis_layout {
    u8 dsfis[28];
    u8   rsv0[4];
    u8 psfis[20];
    u8  rsv1[12];
    u8  rfis[20];
    u8   rsv2[4];
    u8 sdbfis[8];
    u8  ufis[64];
    u8  rsv3[96];

} __attribute__((packed));

typedef struct FIS_H2D {
    u8 type;
    u8 flag;
    u8  cmd;
    u8 rsv0;

    u8 lba0;
    u8 lba1;
    u8 lba2;
    u8  dev;

    u8 lba3;
    u8 lba4;
    u8 lba5;
    u8 rsv1;

    u8 sec_count_low;
    u8 sec_count_hight;

    u8 rsv2[6];

}__attribute__((packed));

u32 init_sata(u16 info[256]);
u8 ahci_sector_read(u64 lba, u16 word[256]);
u8 ahci_sector_write(u64 lba, u16 word[256]);

u32 ahci_mem_base = 0;

#endif
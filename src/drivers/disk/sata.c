#include <drivers/disk/sata.h>
#include <kernel/services.h>
#include <drivers/video/video.h>
#include <drivers/timer/timer.h>

#define FIS_MEM_BASE       0x10000
#define CLB_MEM_BASE       0x20000 

struct HBA_regs* regs;
struct HBA_port_regs* port_regs;

U32 max_port = 0;
U32 cur_port = 0;
U32 ncs = 0;

U32 sata_port_reset(U32 port) {
    port_regs = (struct HBA_port_regs*)(ahci_mem_base + 0x100 + (port * 0x80));

    port_regs->cmd & ~0x01;

    for(U32 timeout = 500; timeout > 0; timeout--) {
        ksleep(1);
        if(!(port_regs->cmd & ~0x8000)) {break;}
    }

    if(regs->ghc & 0x01) {
        video->write_string("Block\n");
    }

    port_regs->sctl = (port_regs->sctl & 0xFFFFFFF8) | 0x01;
    ksleep(5);

    port_regs->sctl &= ~0x07;

    while (!(port_regs->ssts & 0x03));

    port_regs->serr = 0xFFFFFFFF;
    
}

U32 sata_hba_reset() {
    regs->ghc |= 0x01;

    for(U32 timeout = 1000; timeout > 0; timeout--) {
        ksleep(1);
        if(!(regs->ghc & 0x01)) {break;}
    }

    if(regs->ghc & 0x01) {
        video->write_string("Block\n");
    }

    regs->ghc |= 0x80000000;
    
}

U32 init_sata(U16 info[256]) {
    regs = (U32*)ahci_mem_base;

    sata_hba_reset();

    U32 x64_support = 0;
    x64_support = (regs->cap & 0x80000000);

    video->write_string("x64 support: ");

    if(x64_support) {
        video->write_string("Yes\n");
    }else {
        video->write_string("No\n");
    }

    regs->ghc |= 0x80000000;

    U8 port_detect = 0;
    U32 port = 0;

    for(; port < 32; port++) {
        if(regs->pi & (1 << port)) {
            max_port++;
            if(!port_detect) {
                port_detect = 1;
                cur_port = port;
            }
        }
    }

    port_regs = (struct HBA_port_regs*)(ahci_mem_base + 0x100 + (cur_port * 0x80));

    port_regs->cmd &= ~0x01;

    for(U32 timeout = 500; timeout > 0; timeout--) {
        ksleep(1);
        if(!(port_regs->cmd & 0x8000)) {break;}
    }

    if(port_regs->cmd & 0x8000) {
        video->write_string("CR timeout, forcing COMRESET\n");
    }

    port_regs->cmd &= ~0x10;

    for(U32 timeout = 500; timeout > 0; timeout--) {
        ksleep(1);
        if(!(port_regs->cmd & 0x4000)) {break;}
    }

    if(port_regs->cmd & 0x4000) {
        video->write_string("FR timeout, forcing COMRESET\n");
    }

    ncs = (regs->cap >> 8) & 0xF;
    video->write_string("Number of Command Slots: ");
    video->write_int(ncs+1);
    video->write_string("\n");

    service.memory->memset(CLB_MEM_BASE, 0, 1024);
    service.memory->memset(FIS_MEM_BASE, 0, 1024);

    port_regs->clb = CLB_MEM_BASE;
    port_regs->clbu = 0;

    port_regs->fb = FIS_MEM_BASE;
    port_regs->fbu = 0;

    port_regs->cmd |= 0x10;
    port_regs->serr = 0xFFFFFFFF;

    port_regs->sctl = 0x301;
    ksleep(10);

    port_regs->sctl = 0x300;
    ksleep(10);

    port_regs->cmd |= 0x10;     
    port_regs->serr = 0xFFFFFFFF; 

    ksleep(20);

    U32 final_status = 0;

    for(U32 timeout = 500; timeout > 0; timeout--) {
        ksleep(1);
        final_status = port_regs->ssts & 0x0F;
        if(final_status == 0x03) {
            break;
        }
    }

    video->write_string("Real final status: ");
    video->write_int(final_status);
    video->write_string("\n");

    if (final_status == 0x03) {
        video->write_string("0x03!! Success!\n");
        port_regs->cmd |= 0x01;
    }

    return 2;
}
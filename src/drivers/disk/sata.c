#include <drivers/disk/sata.h>
#include <kernel/services.h>
#include <drivers/video/video.h>

#define FIS_MEM_BASE       0x10000
#define CLB_MEM_BASE       0x20000 

struct HBA_regs* regs;
struct HBA_port_regs* port_regs;

U32 init_sata(U16 info[256]) {
    regs = (U32*)ahci_mem_base;

    if(regs->cap & 0x80000000) {
        video->write_string("Supports 64-bit Addressing: Yes\n");
    }else {
        video->write_string("Supports 64-bit Addressing: No\n");
    }

    U32 ncs = (regs->cap >> 8) & 0xF;
    video->write_string("Number of Command Slots: ");
    video->write_int(ncs+1);
    video->write_string("\n");

    U32 np = regs->cap & 0xF;
    video->write_string("Number of Ports: ");
    video->write_int(np+1);
    video->write_string("\n");

    regs->ghc |= 0x80000000;

    U32 port = 0;

    for(;port < 32;port++) {
        if(regs->pi & (1 << port)) {
            break;
        }
    }

    port_regs = (struct HBA_port_regs*)(ahci_mem_base + 0x100 + (port * 0x80));

    video->write_string("Port activity: ");
    video->write_int(port);
    video->write_string("\n");

    video->write_string("Port status: ");

    U8 port_status = (U8)(port_regs->ssts & 0xF);

    if(port_status == 0x03) {
        video->write_string("Device detected, connection established\n");
    }else if(port_status == 0x01) {
        video->write_string("Device detected, but connection not established\n");
    }else {
        video->write_string("Device not detected\n");
    }

    service.memory->memset(FIS_MEM_BASE, 0, 256);
    service.memory->memset(CLB_MEM_BASE, 0, ncs * 32);

   

    return 2;
}
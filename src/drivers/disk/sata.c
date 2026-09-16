#include <drivers/disk/sata.h>
#include <drivers/video/video.h>

struct HBA_regs* regs;

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

    video->write_string("Port activity: ");
    video->write_int(port);
    video->write_string("\n");

    return 2;
}
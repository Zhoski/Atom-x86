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

    port_regs->cmd &= ~0x01;

    while(port_regs->cmd & 0x8000);

    port_regs->cmd &= ~0x10;

    while(port_regs->cmd & 0x4000);

    service.memory->memset(CLB_MEM_BASE, 0, 1024);
    service.memory->memset(FIS_MEM_BASE, 0, 256);

    port_regs->clb = CLB_MEM_BASE;
    port_regs->clbu = 0;

    port_regs->fb = FIS_MEM_BASE;
    port_regs->fbu = 0;

    port_regs->sctl &= 0xFFFFFFF0;
    port_regs->sctl += 0x01; 

    for(U32 tick = 0; tick < 50000; tick++) {
        asm volatile("nop");
    }

    port_regs->sctl &= 0xFFFFFFF0;

    for(U32 tick = 0; tick < 50000; tick++) {
        asm volatile("nop");
    }

    port_regs->cmd |= 0x10000017;

    U32 port_status = port_regs->ssts & 0xF;

    if(port_status == 0x03) {
        video->write_string("Device detected, connection established\n");
    }else if(port_status == 0x01) {
        video->write_string("Device detected, but connection not established\n");
    }else {
        video->write_string("Device not detected\n");
    }

    U32 interface_status = port_regs->ssts & 0x00000F00;

    if(interface_status == 0x00000100) {
        video->write_string("Interface: activity\n");
    }else {
        video->write_string("Interface: sleep\n");
    }

    return 2;
}
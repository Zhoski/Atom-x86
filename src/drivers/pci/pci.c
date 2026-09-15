#include <drivers/pci/pci.h>
#include <drivers/video/video.h>
#include <cpu/io.h>

#define CONFIG_ADDRESS      0xCF8
#define CONFIG_DATA         0xCFC

struct pci_device pci_devices[128];

U32 pci_read(U8 bus, U8 slot, U8 function, U8 offset) {
    U32 data = 0x80000000;
    data |= (U32)(bus << 16);
    data |= (U32)((slot & 0x1F) << 11);
    data |= (U32)((function & 0x07) << 8);
    data |= (U32)(offset & 0xFC);
    
    outl(CONFIG_ADDRESS, data);
    return inl(CONFIG_DATA);
}

void pci_scan_bus0() {
    U32 cur_device = 0;
    for(U32 slot = 0; slot < 31; slot++) {
        for(U32 function = 0; function < 7; function++) {
            U32 data = pci_read(0x00, slot, function, 0x00);
            if(data == 0xFFFFFFFF) {
                continue;
            }else {
                data = pci_read(0x00, slot, function, 0x08);
                pci_devices[cur_device].class = (U8)(data >> 24);
                pci_devices[cur_device].subclass = (U8)((data >> 16) & 0xFF);
                data = pci_read(0x00, slot, function, 0x10);
                pci_devices[cur_device].bar0 = (U32)(data & ~0xFFFFFFFE);

                cur_device++;
            }
        }
    }
}
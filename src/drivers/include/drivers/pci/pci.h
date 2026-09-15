#ifndef PCI_H
#define PCI_H

#include <lib/int.h>

typedef struct pci_device {
    U8 class;
    U8 subclass;
    U32 bar0;
};

U32 pci_read(U8 bus, U8 slot, U8 function, U8 offset);
void pci_scan_bus0();

extern struct pci_device pci_devices[128];

#endif
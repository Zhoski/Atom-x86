#ifndef PIC_H
#define PIC_H
#include <lib/int.h>

void pic_remap();
void pic_irq_mask(U16 port, U8 mask);

#endif

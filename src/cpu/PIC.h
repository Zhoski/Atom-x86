#ifndef __PIC__
#define __PIC__
#include <stdint.h>

void pic_remap();
void pic_irq_mask(uint16_t port, uint8_t mask);

#endif

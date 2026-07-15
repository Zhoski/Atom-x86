#ifndef PIC
#define PIC
#include <stdint.h>
#include "../kernel/int.h"

void pic_remap();
void pic_irq_mask(U16 port, U8 mask);

#endif

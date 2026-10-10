#ifndef HEXDUMP_H
#define HEXDUMP_H

#include <drivers/video/video.h>
#include <lib/int.h>

void hexdump(u32 addres, u32 n) {
    u8* p = (u8*)addres;
    
    for(u32 i = 0; i < n; i++) {
        video->kprintf("------------\nADDRES: %x\nVALUE: %x\n", addres + i, *(p+i));
    }
}

#endif
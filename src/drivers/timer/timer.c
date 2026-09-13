#include <drivers/timer/timer.h>
#include <cpu/cpu.h>
#include <cpu/io.h>

U32 tick = 0;

void init_timer(U32 freequence) {
    U32 divisior = 1193182 / freequence;

    outb(0x43, 0x36);

    U32 low = (U8)(divisior & 0xFF);
    U32 hight = (U8)((divisior >> 8) & 0xFF);

    outb(0x40, low);
    outb(0x40, hight);
}

void ksleep(U32 delay) {
    U32 target = tick + delay;

    while (tick < target)
    {
        halt();
    }
}

void timer_handler() {
    tick++;
}
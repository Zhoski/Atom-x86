#ifndef CPU_H
#define CPU_H

static inline void halt() {
    asm volatile("hlt");
}
static inline void sti() {
    asm volatile("sti");
}

#endif
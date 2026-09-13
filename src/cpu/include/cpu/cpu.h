#ifndef CPU_H
#define CPU_H

static inline void __attribute__((always_inline)) sti(void) {
    asm volatile("sti");
}

static inline void __attribute__((always_inline)) cli(void) {
    asm volatile("cli");
}

static inline void __attribute__((always_inline)) halt(void) {
    asm volatile("hlt");
}


#endif
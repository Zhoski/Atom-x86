#include "PIC.h"
#include <stdint.h>

static inline void outb(uint16_t port, uint8_t data) {
    asm volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t data;
    asm volatile("inb %1, %0" : "=a"(data) : "Nd"(port));
    return data;
}

void pic_remap() {
    uint8_t a1 = inb(0x21); // Сохраняем маски прерываний
    uint8_t a2 = inb(0xA1);

    outb(0x20, 0x11); // ICW1: инициализация
    outb(0xA0, 0x11);

    outb(0x21, 0x20); // ICW2: смещение вектора Master (0x20)
    outb(0xA1, 0x28); // ICW2: смещение вектора Slave (0x28)

    outb(0x21, 0x04); // ICW3: Master знает о Slave на IRQ2
    outb(0xA1, 0x02); // ICW3: Slave знает свой ID

    outb(0x21, 0x01); // ICW4: режим 8086
    outb(0xA1, 0x01);

    outb(0x21, a1);   // Восстанавливаем сохраненные маски
    outb(0xA1, a2);
}

void pic_irq_mask(uint16_t port, uint8_t mask) {
    outb(port, mask);
}

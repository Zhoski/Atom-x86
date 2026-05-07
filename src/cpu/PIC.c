#include "PIC.h"
#include "../kernel/port/io.h"
#include <stdint.h>

// Master PIC
#define PIC1_COMMAND     0x20
#define PIC1_DATA        0x21

// Slave PIC
#define PIC2_COMMAND     0xA0
#define PIC2_DATA        0xA1

// Команды
#define PIC_EOI          0x20 // End of Interrupt (конец прерывания)
#define ICW1_INIT        0x11 // Флаг инициализации

#define ICW2_MASTER_OFFSET 0x20
#define ICW2_SALVE_OFFSET  0x28

void pic_remap() {
    uint8_t a1 = inb(PIC1_DATA); // Сохраняем маски прерываний
    uint8_t a2 = inb(PIC2_DATA);

    outb(PIC1_COMMAND, ICW1_INIT); // ICW1: инициализация
    outb(PIC2_COMMAND, ICW1_INIT);

    outb(PIC1_DATA, ICW2_MASTER_OFFSET); // ICW2: смещение вектора Master (0x20)
    outb(PIC2_DATA, ICW2_SALVE_OFFSET); // ICW2: смещение вектора Slave (0x28)

    outb(PIC1_DATA, 0x04); // ICW3: Подключаем к MASTER SALVE на IRQ2
    outb(PIC2_DATA, 0x02); // ICW3: Slave знает свой ID

    outb(PIC1_DATA, 0x01); // ICW4: режим 8086
    outb(PIC2_DATA, 0x01);

    outb(PIC1_DATA, a1);   // Восстанавливаем сохраненные маски
    outb(PIC2_DATA, a2);
}

void pic_irq_mask(uint16_t port, uint8_t mask) {
    outb(port, mask);
}

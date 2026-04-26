#ifndef __IDT__
#define __IDT__
#include <stdint.h>

// Таблица IDT
struct InterruptDescriptor32 {
    uint16_t base_low;      // Младшая часть адреса обработчика
    uint16_t segment;       // Сегмент селктора
    uint8_t  reserved;      // Зарезервировано
    uint8_t  access;         // Доступ
    uint16_t base_hight;    // Старшая часть адреса обработчика
} __attribute__((packed));

// Указатель на таблицу
struct IDT_pointer {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));                                

void idt_load();
void idt_set(uint8_t index, uint16_t sel, uint8_t access,uint32_t handler);

#endif 

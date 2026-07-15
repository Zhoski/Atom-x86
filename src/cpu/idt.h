#ifndef __IDT__
#define __IDT__
#include <stdint.h>
#include "../kernel/int.h"

// Таблица IDT
struct InterruptDescriptor32 {
    U16 base_low;      // Младшая часть адреса обработчика
    U16 segment;       // Сегмент селктора
    U8  reserved;      // Зарезервировано
    U8  access;        // Доступ
    U16 base_hight;    // Старшая часть адреса обработчика
} __attribute__((packed));

// Указатель на таблицу
struct IDT_pointer {
    U16 limit;
    U32 base;
} __attribute__((packed));                                

void idt_load();
void idt_set(U8 index, U16 sel, U8 access, U32 handler);

#endif 

#include "idt.h"

struct InterruptDescriptor32 IDT[256]; // Таблица на 256 прерываний

void idt_load() {
   struct IDT_pointer idt_ptr;
   idt_ptr.limit = (uint16_t)(sizeof(struct InterruptDescriptor32) * 256) - 1;
   idt_ptr.base  = (uint32_t)&IDT;

    asm volatile("lidt (%0)" : : "r"(&idt_ptr));
}

void idt_set(uint8_t index, uint16_t sel, uint8_t access, uint32_t handler) {
    IDT[index].base_low = handler & 0xFFFF;
    IDT[index].base_hight = (handler >> 16) & 0xFFFF;

    IDT[index].segment = sel;   
    IDT[index].reserved = 0;       
    IDT[index].access = access;       
}


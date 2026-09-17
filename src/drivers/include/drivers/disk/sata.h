#ifndef SATA_H
#define SATA_H

#include <lib/int.h>

typedef struct HBA_port_mem;

// Структура регистров одного порта (занимает ровно 0x80 байт)
struct HBA_port_mem {
    U32 clb;       // 0x00, Базовый адрес списка команд (младшие 32 бита)
    U32 clbu;      // 0x04, Базовый адрес списка команд (старшие 32 бита)
    U32 fb;        // 0x08, Базовый адрес FIS (младшие 32 бита)
    U32 fbu;       // 0x0C, Базовый адрес FIS (старшие 32 бита)
    U32 is;        // 0x10, Статус прерываний порта
    U32 ie;        // 0x14, Разрешение прерываний порта
    U32 cmd;       // 0x18, Команды и статус порта
    U32 rsv0;      // 0x1C, Резерв
    U32 tfd;       // 0x20, Task File Data (Статус и Ошибка)
    U32 sig;       // 0x24, Сигнатура (Тип устройства)
    U32 ssts;      // 0x28, Serial ATA Status (SStatus)
    U32 sctl;      // 0x2C, Serial ATA Control (SControl)
    U32 serr;      // 0x30, Serial ATA Error (SError)
    U32 sact;      // 0x34, Serial ATA Active (SActive)
    U32 ci;        // 0x38, Command Issue (Запуск команд)
    U32 sntf;      // 0x3C, Serial ATA Notification
    U32 fbs;       // 0x40, FIS-based Switching Control
    U32 devslp;    // 0x44, Device Sleep
    U8  rsv1[56];  // 0x48 - 0x7F, Резерв до конца 0x80-байтовой структуры порта
} __attribute__((packed));

// Глобальная структура AHCI памяти (Generic Host Control)
struct HBA_mem {
    // 0x00 - 0x2C: Глобальные регистры HBA
    U32 cap;       // 0x00, Host Capabilities
    U32 ghc;       // 0x04, Global Host Control
    U32 is;        // 0x08, Interrupt Status
    U32 pi;        // 0x0C, Ports Implemented (Маска портов)
    U32 vs;        // 0x10, Version (Версия спецификации)
    U32 ccc_ctl;   // 0x14, Command Completion Coalescing Control
    U32 ccc_pts;   // 0x18, Command Completion Coalescing Ports
    U32 em_loc;    // 0x1C, Enclosure Management Location
    U32 em_ctl;    // 0x20, Enclosure Management Control
    U32 cap2;      // 0x24, Host Capabilities Extended
    U32 bohc;      // 0x28, BIOS/OS Handoff Control and Status

    // 0x2C - 0x9F: Резервная зона по спецификации Intel AHCI
    U8  rsv[116];  

    // 0xA0 - 0xFF: Регистры для вендоров (Vendor Specific)
    U8  vendor[96]; 

    // 0x100: Начало портов. Каждый занимает 0x80 байт. Итого 32 * 0x80 = 0x1000 байт.
    struct HBA_port_mem port[32]; 
} __attribute__((packed));

typedef struct sata_disk {
    
};

U32 init_sata(U16 info[256]);
U8 sata_read_sector(U32 lba, U16 word[256]);
U8 sata_write_sector(U32 lba, U16 word[256]);

U32 ahci_mem_base = 0;

#endif
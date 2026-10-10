#ifndef REQUEST_H
#define REQUEST_H

#include "type.h"

typedef struct request request_t;

struct request
{
    /**
     * 0 бит (-c)  - Создать диск
     * 1 бит (-p)  - Упаковать файл на диск
     * 2 бит (-b)  - Установить загрузочный сектор
     * 3 бит (-mb) - Установить основной загрузчик
     * 4 бит (-K)  - Установить размер диска в кб
     * 5 бит (-M)  - Установить размер диска в мб
     * 6 бит (-o)  - Установить выходной файл
     * 7 бит (-f)  - Установить флаг для файла
     * 8 бит (-d)  - Установить диск
     * 9 бит (-bs) - Установить размер блока
     */
    u16 flag0;
    u8 rsv0[2];

    u32 disk_size;
    u32 block_size;

    u8* boot;
    u8* main_boot;
    
    u8* disk;

    u8* out_path;
    u8** argv;
    u32  argc;
}; 


#endif
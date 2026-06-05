#ifndef FILE
#define FILE

typedef struct
{
    unsigned int size;      // Размер в байтах
    unsigned char* buffer;  // Начало файла
    unsigned char* ptr;     // Текущее смещение в файле
    unsigned int eoi;       // Конец файла
} File;

unsigned char cat(const unsigned char* file_name, unsigned char buffer[512]);

#endif
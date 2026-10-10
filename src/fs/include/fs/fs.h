#ifndef FS
#define FS
#include <lib/int.h>

#define AFS_T       0x01
#define FLUIDFS_T   0x02

typedef struct File;

typedef struct FileSystem
{
    u32* (*check)(const u8 *__restrict__ file_name); // Возвращает 1 в случаи если файл есть и 0 в ином
    u8 (*open)(const u8 *__restrict__ file_name);
    u8 (*read)(const u8 *__restrict__ file_name, u32 n,u8 *__restrict__ out);
    u8 (*create)(const u8 *__restrict__ file_name, u16 size);
    u8 (*delete)(const u8 *__restrict__ file_name);
    u8 (*update) (const u8 *__restrict__ file_name, u8* in, u32 bytes);
    u8 (*get_root) (u8 *__restrict__ out);
    u32 (*init)();
} FileSystem_t;

u32 init_fs(u32 type);

extern FileSystem_t* fs;

#endif

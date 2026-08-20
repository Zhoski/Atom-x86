#ifndef FS
#define FS
#include <stdint.h>
#include "../../kernel/int.h"

typedef struct File;

typedef struct
{
    U32* (*check)(const U8 *__restrict__ file_name);
    U8 (*open)(const U8 *__restrict__ file_name);
    U8 (*read)(const U8 *__restrict__ file_name, U32 n,U8 *__restrict__ out);
    U8 (*create)(const U8 *__restrict__ file_name, U16 size);
    U8 (*delete)(const U8 *__restrict__ file_name);
    U8 (*update) (const U8 *__restrict__ file_name, U8* in, U32 bytes);
    U8 (*get_root) (U8 *__restrict__ out);
    U8 (*init)();
} FileSystem;

void init_fs();

extern FileSystem* fs;

#endif

#ifndef __AFS__
#define __AFS__
#include <stdint.h>
#include "../../kernel/int.h"

typedef struct __attribute__((packed)) {
    U8 name[8];
    U8 ext[3];
    U16 start_sec;
    U16 size;
    U8 flags;
} File;

U32* afs_check_file(const U8 *__restrict__ file_name);
U8 afs_open(const U8 *__restrict__ file_name);
U8 afs_read(const U8 *__restrict__ file_name, U8 *__restrict__ out);
U8 afs_create(const U8 *__restrict__ file_name, U16 size);
U8 afs_delete(const U8 *__restrict__ file_name);
U8 afs_update(const U8 *__restrict__ file_name, U8 *__restrict__ in, U32 bytes);
U8 afs_get_root(U8 *__restrict__ out);
U8 afs_init();

extern File _file;

#endif

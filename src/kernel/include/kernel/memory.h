#ifndef __MEMORY__
#define __MEMORY__
#include <lib/int.h>

void memcpy(void* __restrict__ __from,void* __restrict__ __in, U32 __c);
U8 memread(U8* __restrict__ from);
U16 memread_dw(U16* __restrict__ from);
void memset(void* __restrict__ in, U8 v, U32 size);
U32 memread_dd(U32* __restrict__ from);
U8* malloc(U32 size);
void free(U8* __restrict__ ptr);
void create_heap();
void init_memory();
#endif

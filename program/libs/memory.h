#ifndef __MEMORY__
#define __MEMORY__

unsigned char memread(unsigned char* from);
unsigned short memread_dw(unsigned short* from);
unsigned int memread_dd(unsigned int* from);
void memset(unsigned char* in, unsigned char v, unsigned int size);
void memcpy(void* from,void* in, unsigned int size);
unsigned char* malloc(unsigned int size);
unsigned char free(unsigned char* data);

#endif
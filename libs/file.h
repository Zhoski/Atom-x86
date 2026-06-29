#ifndef FILE
#define FILE

unsigned int sys_check(const unsigned char* __restrict__ __file);
unsigned int sys_read(const unsigned char* __restrict__ __file, unsigned char* __restrict__ __out);
unsigned int sys_write(const unsigned char* __restrict__ __file, unsigned char* __restrict__ __in, unsigned int __c);
unsigned char sys_create(const unsigned char* __restrict__ __file);
unsigned char sys_delete(const unsigned char* __restrict__ __file);
unsigned int sys_run(const unsigned char* __restrict__ file);
unsigned int sys_died();
unsigned char get_root(unsigned char *__restrict__ out);

#endif
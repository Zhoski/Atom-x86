#ifndef FILE
#define FILE

unsigned char cat(const unsigned char* file_name, unsigned char *out);
unsigned char get_root(unsigned char *__restrict__ out);
unsigned int sys_run(unsigned char* __restrict__ file);
unsigned int sys_died();
#endif
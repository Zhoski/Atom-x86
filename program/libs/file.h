#ifndef FILE
#define FILE

#define FWRITE      0x00
#define FREAD       0x01

#define BASE        0x02
#define END         0x03

typedef struct {
    unsigned int bytes;
    unsigned char* cur;
    unsigned char base[];
} File;

File* fopen(unsigned char *__restrict__ file, const unsigned int mode);
void fclose(File* stream);
void fread(File* stream, unsigned int n, unsigned char* out);

unsigned int sys_check(const unsigned char* __restrict__ __file);
unsigned int sys_read(const unsigned char* __restrict__ __file, unsigned char* __restrict__ __out);
unsigned int sys_write(const unsigned char* __restrict__ __file, unsigned char* __restrict__ __in, unsigned int __c);
unsigned char sys_create(const unsigned char* __restrict__ __file);
unsigned char sys_delete(const unsigned char* __restrict__ __file);
unsigned int sys_run(const unsigned char* __restrict__ file);
unsigned int sys_died();
unsigned char get_root(unsigned char *__restrict__ out);

#endif

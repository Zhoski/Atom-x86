#ifndef __LIBC__
#define __LIBC__

typedef unsigned char   uint8;
typedef unsigned short uint16;
typedef unsigned int   uint32;

void SetFGColor(unsigned char color);
void SetBGColor(unsigned char color);
void printf(const char *format, ...);
void putchar(unsigned char c);
void clear_screen(unsigned char color);
uint8 get_char();
void set_cursor(const unsigned short x, const unsigned short y);
void get_cursor(unsigned int* x, unsigned int* y);
#endif
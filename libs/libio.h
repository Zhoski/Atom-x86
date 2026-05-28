#ifndef __LIBC__
#define __LIBC__

typedef unsigned char   uint8;
typedef unsigned short uint16;
typedef unsigned int   uint32;

void SetFGColor(unsigned char r,unsigned char g,unsigned char b);
void SetBGColor(unsigned char r,unsigned char g,unsigned char b);
void printf(unsigned char s[]);
void putchar(unsigned char c);
void clear_screen(unsigned char r, unsigned char g, unsigned char b);
uint8 get_char();

#endif
#ifndef __SERVICES__
#define __SERVICES__
#include <stdint.h>

typedef struct {
    void (*write_string)(const char *data);
    void (*clear)(void);
    void (*write_char)(const char data);
    void (*set_cursor_position)(uint8_t, uint8_t);
    void (*set_attribute)(uint8_t, uint8_t);
    void (*write_int)(int);
    void (*write_hex)(int, int);
} vga;

typedef struct {
   void (*memcpy)(uint8_t*, uint8_t*, uint32_t);
   uint8_t (*memread)(uint8_t*);
   uint32_t (*memread_dd)(uint32_t*);
} memory;

typedef struct {
   uint32_t (*malloc)(); 
} pagging;

typedef struct {
    uint32_t (*malloc)();
} stack;

typedef struct {
   pagging* pagging;
   stack*   stack;
} allocate;

typedef struct {
    const char name[16];
    vga* vga;
    memory* memory;
    allocate* alocator;
} services;

extern services service;

#endif

#ifndef __PROCESS__
#define __PROCESS__
#define MAX_PROCESS     128
#define USED            1
#define FREE            0
#define SUCCESS         0
#define FULL            1
#include <stdint.h>

typedef struct {
    uint32_t entry;
    uint32_t stack_entry;
    uint32_t stack_ptr;
    uint8_t flag;
}PBC;

typedef PBC process;

extern uint8_t process_spawn(uint32_t entry, uint32_t entry_stack);

#endif

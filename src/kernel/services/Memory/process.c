#include "process.h"
#include "memory_map.h"

process process_table[MAX_PROCESS];

uint8_t process_spawn(uint32_t entry, uint32_t entry_stack) {
    for(uint32_t i = 0;i < MAX_PROCESS;i++) {
        if(!process_table[i].flag) {
            process_table[i].entry = entry;
            process_table[i].stack_entry = entry_stack + 0x2000;
            process_table[i].stack_ptr = entry_stack + 0x2000;
            process_table[i].flag = USED;
            return SUCCESS;
        }
    }

    return FULL;
}

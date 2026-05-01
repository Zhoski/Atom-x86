#include "process.h"
#include "../../../drivers/vga.h"
#include "memory_map.h"

process process_table[MAX_PROCESS];

uint8_t process_spawn(uint32_t entry, uint32_t entry_stack) {
    for(uint32_t i = 0;i < MAX_PROCESS;i++) {
        if(!process_table[i].flag) {
            process_table[i].PID = i;
            process_table[i].entry = entry;
            process_table[i].stack_entry = entry_stack + 0x2000;
            process_table[i].stack_ptr = entry_stack + 0x2000;
            process_table[i].flag = USED;

            //kwrite_string("Create new process PID: ");
            //kwrite_int(process_table[i].PID);
            //kwrite_string("\nEntry: ");kwrite_int(process_table[i].entry);
            //kwrite_string("\nStack: ");kwrite_int(process_table[i].stack_entry);
            //kwrite_string("\n");
            return SUCCESS;
        }
    }

    return FULL;
}

uint8_t process_dead(uint32_t pid);

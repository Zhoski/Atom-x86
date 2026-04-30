#include "memory_map.h"
#include "process.h"
#include <stdint.h>

void program_execute(uint32_t entry) {  
    asm volatile (     
        "pushl $0x08 \n\t"   
        "pushl %0   \n\t"     
        "lretl"                    
        : 
        : "a" (entry)      
    ); 
}

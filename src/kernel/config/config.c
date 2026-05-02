#include "config.h"
#include "../../drivers/VGA/vga.h"

void init_config() {
    for(uint8_t i = 0;i < USER_NAME_MAX;i++) {
        if(memread(CONFIG_USER_OFFSET + i) == 0) {
            break;
        }
        user_name[i] = memread(CONFIG_USER_OFFSET + i);  
    }
    for(uint8_t i = 0;i < USER_NAME_MAX;i++) { 
        user_pass[i] = memread(CONFIG_PASS_OFFSET + i);  
    }
}

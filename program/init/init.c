#include "../libs/file.h"
#include "../libs/strio.h"

void main() {
    if(!sys_check("init.cfg")) {
        printf("%[12INIT.BIN ERROR FILE INIT.CFG NOT FOUND%[15");
        for(;;) {
            asm("hlt");
        }
    }else {
        char init_cfg[512];
        sys_read("init.cfg", init_cfg);
        if(!sys_check("USER    CFG")) {
            sys_run("SETUP   BIN");
        }else {
            sys_run("SHELL   BIN");
        }
    }
}

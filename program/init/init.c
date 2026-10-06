#include <slib/string.h>
#include <slib/file.h>
#include <slib/strio.h>

void main() {
    clear_screen(0);
    printf("INIT.BIN work\n");

    uint32 status = sys_check("init.cfg");

    if(status) {
        printf("init.cfg found\n");
    }

    sys_run("setup.bin");

    for(;;) {
        asm("hlt");
    }
    /*if(!sys_check("init.cfg")) {
        printf("%[12INIT.BIN ERROR FILE INIT.CFG NOT FOUND%[15");
        for(;;) {
            asm("hlt");
        }
    }else {
        char init_cfg[512];
        sys_read("init.cfg", 512,init_cfg);
        if(!sys_check("USER    CFG")) {
            sys_run("SETUP   BIN");
        }else {
            sys_run("SHELL   BIN");
        }
    }*/
}

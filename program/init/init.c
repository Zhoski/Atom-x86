#include <slib/string.h>
#include <slib/file.h>
#include <slib/strio.h>

void main() {
    clear_screen(0);
    printf("INIT.BIN work\n");

    uint32 status = sys_check("/home/init.cfg");

    if(status) {
        printf("init.cfg found\n");
    }

    if(!sys_check("/home/init.cfg")) {
        printf("%[12INIT.BIN ERROR FILE INIT.CFG NOT FOUND%[15");
        for(;;) {
            asm("hlt");
        }
    }else {
        char init_cfg[512];
        sys_read("/home/init.cfg", 512,init_cfg);
        if(!sys_check("/home/user.cfg")) {
            sys_run("/system/setup.bin");
        }else {
            sys_run("/home/shell.bin");
        }
    }
}

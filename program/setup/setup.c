#include "../../libs/file.h"
#include "../../libs/strio.h"

void main() {
    clear_screen(0);
    SetBGColor(1);
    SetFGColor(15);
    printf("SETUP.BIN");
    set_cursor(9,0);
    for(uint32 i = 9;i < 80;i++) {
        putchar(' ');
    }
    set_cursor(0,29);
    for(uint32 i = 0;i < 80;i++) {
        putchar(' ');
    }

    sys_create("user.cfg");
    sys_write("user.cfg", "user:Zhoski",12);
    set_cursor(0,1);
    char c = 0;

    while (1)
    {
        c = get_char();
        if(c) {
            sys_run("SHELL   BIN");
        }
    }
}
#include "../../libs/file.h"
#include "../../libs/strio.h"

void main() {
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

    sys_create("USER    CFG");
    sys_write("USER    CFG", "user:Zhoski",12);
    set_cursor(0,1);
    char c = 0;

    while (1)
    {
        c = get_char();
        if(c) {
            putchar(c);
            sys_run("SHELL   BIN");
        }
    }
}
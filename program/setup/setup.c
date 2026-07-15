#include "../libs/file.h"
#include "../libs/strio.h"
#include "../libs/types.h"

void main() {
    clear_screen(0);

    char username[32] = {0};

    sys_create("user.cfg");

    printf("SETUP.BIN");

    set_cursor(30, 11);
    printf("Enter your username");

    set_cursor(24,12);
    printf("\xC9\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBB");
    set_cursor(24,13);
    printf("\xBA ____________________________ \xBA");
    set_cursor(24,14);
    printf("\xC8\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBC");

    set_cursor(26,13);

    char c = 0;
    int i = 0;

    while (1)
    {
        c = get_char();
        if(c == 0x0A) {
            char data[37];

            data[0] = 'u';
            data[1] = 's';
            data[2] = 'e';
            data[3] = 'r';
            data[4] = ':';

            for(int j = 0;j < 32;j++) {
                data[5 + j] = username[j];
            }

            sys_write("user.cfg", data, 37);

            sys_run("SHELL   BIN");
        }else if(c) {
            if(c == 0x08 && i != 0) {
                U32 x;
                U32 y;
                get_cursor(&x, &y);
                set_cursor(x - 1, y);
                putchar(' ');
                set_cursor(x - 1, y);
                i--;
                username[i] = 0;
                continue;
            }

            if(c != 0x01) {
                putchar(c);
                username[i] = c;
                i++;
            }
        }
    }
}

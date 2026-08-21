#include "../libs/strio.h"
#include "../libs/string.h"
#include "../libs/file.h"
#include "../libs/memory.h"
#include "../libs/types.h"

#define SHIFT 0x01
#define CAPS  0x02
#define CTRL  0x03
#define ENTER 0x0A 
#define BACKSPACE 0x08 
#define ESC       0x1B

#define BUFFER_SIZE 4 * 1024

U8* buffer;
U32 buffer_index = 0;

void draw_header() {
    clear_screen(0);
    SetFGColor(1);

    for(int i = 0;i < 80;i++) {
        printf("\xDB");
    }

    set_cursor(0, 29);
    for(int i = 0;i < 80;i++) {
        printf("\xDB");
    }

    set_cursor(0, 0);
    SetFGColor(15);
    SetBGColor(1);
    printf("NotePad v1.0");

    set_cursor(0, 29);
    printf("F1 - open   F2 - save   ESC - exit");

    SetBGColor(0);
    set_cursor(0, 1);
}

void file_open() {
    set_cursor(30, 11);

    set_cursor(24,12);
    printf("\xC9\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBB");
    set_cursor(24,13);
    printf("\xBA        Enter file name       \xBA");
    set_cursor(24,14);
    printf("\xBA ____________________________ \xBA");
    set_cursor(24,15);
    printf("\xC8\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBC");
    U8* fname[11];
    U32 findex = 0;
    set_cursor(26,14);
    while (1)
    {
        U8* c = get_char();
        if(c) {
            if(c != ENTER && findex < 11) {
                if(c != CAPS && c != SHIFT) {
                    putchar(c);
                    fname[findex] = c;
                    findex++;
                }
            }else {
                draw_header();
                File* file = fopen("license.txt", FREAD);
                if(!file) {
                    printf("File [%s] not found", fname);
                    return;
                }

                buffer_index = file->bytes;
                memset(buffer, 0, BUFFER_SIZE);

                memcpy(file->base, buffer, file->bytes);
                printf(buffer);
                fclose(file);
                break;
            }
        }
    }
}

void loop() {
    while (1)
    {
        char c = get_char();
        if(c) {
            if(c == BACKSPACE) {
                if(buffer_index != 0) {
                    U32 x;
                    U32 y;
                    get_cursor(&x, &y);
                    if(x > 0) {
                        set_cursor(x - 1, y);
                        putchar(' ');
                        set_cursor(x - 1, y);
                    }else {
                        U32 new_x = 79;
                        U32 new_y = y - 1;

                        set_cursor(new_x, new_y);
                        putchar(' ');
                        set_cursor(new_x, new_y);
                    }
                    buffer_index--;
                    buffer[buffer_index] = 0;
                }
                continue;
            }
            else if(c == 0x11) {
                file_open();
            }
            else if(c == ESC) {
                free(buffer);
                sys_died();
            }
            else if(c != CAPS && c != SHIFT) {
                buffer[buffer_index] = c;
                buffer_index++;
                putchar(c);
            }
        }
    }
}

void main() {
    buffer = malloc(BUFFER_SIZE);
    memset(buffer, 0, BUFFER_SIZE);
    draw_header();

    loop();
}
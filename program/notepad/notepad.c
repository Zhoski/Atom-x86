#include <slib/strio.h>
#include <slib/string.h>
#include <slib/file.h>
#include <slib/memory.h>
#include <slib/types.h>

#define SHIFT     0x01
#define CAPS      0x02
#define CTRL      0x03
#define ENTER     0x0A 
#define BACKSPACE 0x08 
#define ESC       0x1B

#define BUFFER_SIZE 4 * 1024

U8* buffer;
U8 cur_file[11];
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
    printf("NotePad v1.0    A:/%s",cur_file);

    set_cursor(0, 29);
    printf("F1 - open   F2 - save   ESC - exit");

    SetBGColor(0);
    set_cursor(0, 1);
}

void get_file_name() {
    set_cursor(30, 11);

    set_cursor(24,12);
    printf("\xC9\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBB");
    set_cursor(24,13);
    printf("\xBA        Enter file name       \xBA");
    set_cursor(24,14);
    printf("\xBA ____________________________ \xBA");
    set_cursor(24,15);
    printf("\xC8\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBC");
    U32 findex = 0;

    memset(cur_file, 0, 11);

    set_cursor(26,14);
    while (1)
    {
        U8* c = get_char();
        if(c) {
            if(c != ENTER && findex < 11) {
                if(c != CAPS && c != SHIFT) {
                    putchar(c);
                    cur_file[findex] = c;
                    findex++;
                }
            }
            else {
                break;
            }
        }
    }
}

void file_open() {
    get_file_name();

    draw_header();
    File* f = fopen(cur_file, FREAD);
    if(!f) {
        printf("%[12File [%s] not found%[15", cur_file);
        return;
    }

    memset(buffer, 0, BUFFER_SIZE);
    
    fread(f, f->bytes, buffer);

    buffer_index = f->bytes;

    fclose(f);

    printf("%s",buffer);
}

void file_save() {
    get_file_name();
    draw_header();

    for(U32 i = 0;i < buffer_index;i++) {
        putchar(buffer[i]);
    }

    File* f = fopen(cur_file, FWRITE);
    if(!f) {
        sys_create(cur_file, BUFFER_SIZE);
        f = fopen(cur_file, FWRITE);
        if(!f) {
            printf("%[12File create error%[15");
        }
    }

    fwrite(f, buffer_index, buffer);
    fclose(f);
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
            else if(c == 0x12) {
                file_save();
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
    memset(cur_file, 0, 11);
    draw_header();

    loop();
}
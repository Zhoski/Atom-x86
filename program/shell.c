#include "../libs/strio.h"
#include "../libs/string.h"
#include "../libs/file.h"
#include "../libs/memory.h"

#define COMMAND_BUFFER_SIZE    128
#define COMMAND_COUNT            6

#define SHIFT 0x01
#define CAPS  0x02
#define CTRL  0x03
#define ENTER 0x0A 
#define BACKSPACE 0x08 

typedef struct __attribute__((packed)) {
    uint8 name[8];
    uint8 ext[3];
    uint16 start_sec;
    uint16 size;
    uint8 flags;
} File;

typedef struct Command
{
    uint8* cmd_name;
    void (*handler)();
};

void help();
void clear(uint8 color);
void show_license();
void dir();
void run();

struct Command cmd[] = {
    {"help", help},
    {"clear", clear},
    {"license", show_license},
    {"dir", dir},
    {"run", run},
    {"exit", sys_died}
};
uint32 command_index = 0;
uint8 *command_buffer;

uint8* info =   "\nAtom interactive shell %[13v 0.1%[15\n"
                    "Copyright (c) 2026 Zhoski. Licensed under the MIT License.\n\n"
                    "Type %[14\"help\"%[15 or %[14\"license\"%[15 for more information.\n\n";

void help() {
    uint8* help_msg =   "\nclear -- clear screen\n"
                        "license -- show license\n"
                        "dir -- displays all files in a directory";
    printf(help_msg);
}

void clear(uint8 color) {
    clear_screen(color);

    printf(info);
}

void show_license() {
    printf("\n");
    char *file;
    uint32 status = cat("LICENSE TXT",file);

    if(status == 0) {
        printf("%s\n",file);
    }else {
        printf("%[12LICENSE.TXT not found%[15");
    }
}

void run() {
    sys_run("HELLO   BIN");
}

void dir() {
    printf("\n\n/root:\n");
    uint8* root = malloc(8192);
    get_root(root);
    File file;
    
    while (*root)
    {
        int i = 0;
        int j = 0;
        memcpy(root, (uint8*)&file, 16);
        while (file.name[i] != ' ')
        {
            putchar(file.name[i]);
            i++;
            j++;
        }
        putchar('.');

        i = 0;

        while (i < 3)
        {
            putchar(file.ext[i]);
            i++;
            j++;
        }

        while (j != 11)
        {
            putchar(' ');
            j++;
        }

        printf("      %[03%d%[15",file.size);
        
        putchar('\n');

        root += 16;
    }   

    free(root);
}

void execute() {
    command_buffer[command_index] = '\0';
    if(!command_buffer[0]) {
        command_index = 0;
        return;
    }
    uint32 is_found = 0;
    for(uint32 i = 0;i < COMMAND_COUNT;i++) {
        if(strcmp(command_buffer, cmd[i].cmd_name) == 0) {
            cmd[i].handler();
            is_found = 1;
        }
    }

    for(uint32 i = 0;i < COMMAND_BUFFER_SIZE;i++) {
        command_buffer[i] = 0;
    }

    if(!is_found) {
        printf("\n%[12Atom: Unknown command%[15");
    }

    command_index = 0;
}

void shell_main() {
    printf(info);
    printf("%[10root> %[15");
    for(uint32 i = 0;i < COMMAND_BUFFER_SIZE;i++) {
        command_buffer[i] = 0;
    }

    uint8 c = '\0';
    while (1)
    {
        c = get_char();
        if(c) {
            if(c == ENTER) {
                execute();
                printf("\n%[10root> %[15");
            }else if(c == BACKSPACE) {
                uint32 x;
                uint32 y;
                get_cursor(&x, &y);
                set_cursor(x - 1, y);
                putchar(' ');
                set_cursor(x - 1, y);
                command_index--;
                command_buffer[command_index] = 0;
            }else {
                if (command_index < COMMAND_BUFFER_SIZE - 1) {
                    command_buffer[command_index++] = c;
                    putchar(c);
                }
            }
        }
    }
}

void main() {
    command_buffer = malloc(COMMAND_BUFFER_SIZE);
    shell_main();
}
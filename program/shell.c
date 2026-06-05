#include "../libs/strio.h"
#include "../libs/string.h"
#include "../libs/file.h"
#include "../libs/memory.h"

#define COMMAND_BUFFER_SIZE     64
#define COMMAND_COUNT            3

#define SHIFT 0x01
#define CAPS  0x02
#define CTRL  0x03
#define ENTER 0x0A 
#define BACKSPACE 0x08 

typedef struct Command
{
    uint8* name;
    void (*handler)();
};

typedef struct Command command_list[COMMAND_COUNT];
command_list cmd;
uint32 command_index = 0;
uint8 command_buffer[COMMAND_BUFFER_SIZE];

void help() {
    uint8* help_msg =   "\nclear -- clear screen\n"
                        "license -- show license\n"
                        "dir -- displays all files in a directory";
    printf(help_msg);
}

void clear(uint8 color) {
    clear_screen(color);
}

void show_license() {
    char *file;
    uint32 status = cat("LICENSE TXT",file);
    
    if(status == 0) {
        printf("\n%s",file);
    }else if(status == 1) {
        SetFGColor(12);
        printf("%[12\nFILE NOT FOUND%[15");
        SetFGColor(15);
    }
}

void execute() {
    command_buffer[command_index] = '\0';
    if(!command_buffer[0]) {
        command_index = 0;
        return;
    }
    uint8 is_found = 0;
    for(uint32 i = 0;i < COMMAND_COUNT;i++) {
        if(strcmp(command_buffer, cmd[i].name) == 0) {
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
    uint8* info =   "\nAtom interactive shell %[13v 0.1%[15\n"
                    "Copyright (c) 2026 Zhoski. Licensed under the MIT License.\n\n"
                    "Type \"help\" or \"license\" for more information.\n\n";
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
                SetFGColor(10);
                printf("\n%[10root> %[15");
                SetFGColor(15);
            }else {
                if (command_index < COMMAND_BUFFER_SIZE - 1) {
                    command_buffer[command_index++] = c;
                    putchar(c);
                }
            }
        }
    }
}

void AddCommand(command_list cmd, uint32 i, uint8* name, void (*handler)()) {
    cmd[i].name = name;
    cmd[i].handler = handler;
}

void main() {
    AddCommand(cmd, 0, "help", help);
    AddCommand(cmd, 1, "clear", clear);
    AddCommand(cmd, 2, "license", show_license);

    shell_main();
}
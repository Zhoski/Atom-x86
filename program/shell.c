#include "../libs/libio.h"
#include "../libs/string.h"

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

void clear() {
    clear_screen(0,0,0);
}

void show_license() {
    const char *license = 
        "\nMIT License\n"
        "Copyright (c) 2026 Zhoski\n\n"
        "Permission is hereby granted, free of charge, to any person obtaining a copy\n"
        "of this software and associated documentation files (the \"Software\"), to deal\n"
        "in the Software without restriction, including without limitation the rights\n"
        "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\n"
        "copies of the Software, and to permit persons to whom the Software is\n"
        "furnished to do so, subject to the following conditions:\n\n"
        "The above copyright notice and this permission notice shall be included in all\n"
        "copies or substantial portions of the Software.\n\n"
        "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n"
        "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
        "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n"
        "AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n"
        "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n"
        "OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\n"
        "SOFTWARE.\n";
    
    printf(license);
}

void execute() {
    command_buffer[command_index] = '\0';
    if(!command_buffer[0]) {
        command_index = 0;
        return;
    }
    uint8 *not_found_msg = "\nAtom: Unknown command";
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
        SetFGColor(255, 0, 0);
        printf(not_found_msg);
        SetFGColor(255,255,255);
    }

    command_index = 0;
}

void shell_main() {
    uint8* info =   "\nAtom interactive shell $[13v 0.1$[15\n"
                    "Copyright (c) 2026 Zhoski. Licensed under the MIT License.\n\n"
                    "Type \"help\" or \"license\" for more information.\n\n";
    printf(info);
    SetFGColor(0,255,0);
    printf("root");
    printf("> ");
    SetFGColor(255,255,255);
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
                SetFGColor(0,255,0);
                printf("\nroot");
                printf("> ");
                SetFGColor(255,255,255);
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
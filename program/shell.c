#include "../libs/strio.h"
#include "../libs/string.h"
#include "../libs/file.h"
#include "../libs/memory.h"

#define COMMAND_BUFFER_SIZE    128
#define COMMAND_COUNT           10

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

void cmd_help();
void cmd_clear();
void cmd_dir();
void cmd_run();
void cmd_read();
void cmd_write();
void cmd_create();
void cmd_delete();
void cmd_check();

struct Command cmd[] = {
    {"help", cmd_help},
    {"clear", cmd_clear},
    {"dir", cmd_dir},
    {"run", cmd_run},
    {"exit", sys_died},
    {"read", cmd_read},
    {"write", cmd_write},
    {"c", cmd_create},
    {"del", cmd_delete},
    {"is",cmd_check},
};
uint32 command_index = 0;
uint8 *command_buffer;

uint32 *argv_buff;
uint8** argv;

uint8* info =   "\nAtom interactive shell %[13v 0.1%[15\n"
                    "Copyright (c) 2026 Zhoski. Licensed under the MIT License.\n\n"
                    "Type %[14\"help\"%[15 or %[14\"license\"%[15 for more information.\n\n";

uint8 user[32];
uint8 pass[32];

void cmd_help() {
    uint8* help_msg =   "\n"
                        "%[11[BASE]%[15\n"
                        "   clear                -- clear screen\n"
                        "%[11[DISK]%[15\n"
                        "   c      [file]        -- creates file\n"
                        "   del    [file]        -- deletes file\n"
                        "   read   [file]        -- read file\n"
                        "   write  [file] [data] -- write in file\n"
                        "   dir                  -- displays all files in a directory\n"
                        "%[11[SYSTEM]%[15\n"
                        "   sys                  -- show system info\n"
                        "   setdrv [type] [file] -- change driver\n"
                        "   getdrv               -- show list cur drv";
    printf(help_msg);
}

void cmd_clear() {
    clear_screen(0);

    printf(info);
}

void cmd_read() {
    printf("\n");
    char *buff;
    uint32 status = sys_read(argv[1],buff);

    if(status == 0) {
        printf("%s",buff);
    }else {
        printf("%[12%s not found%[15", argv[1]);
    }
}

void cmd_check() {
    int ret = sys_check(argv[1]);
    if(ret) {
        printf("\n%[12%s not found%[15", argv[1]);
    }else {
        printf("\n%[10%s found%[15", argv[1]);
    }
}

void cmd_write() {
    int size = 0;
    while (argv[2][size] != 0)
        size++;
    
    int ret = sys_write(argv[1], argv[2], size);
    if(ret) {
        printf("\n%[12%s not found%[15", argv[1]);
    }
}

void cmd_delete() {
    int ret = sys_delete(argv[1]);
    if(ret) {
        printf("\n%[12%s not found%[15", argv[1]);
    }else {
        printf("\n%[10%s succes deleted%[15", argv[1]);
    }
}

void cmd_create() {
    sys_create(argv[1]);
    printf("\n%[10%s succes created%[15", argv[1]);
}

void cmd_run() {
    uint32 status = sys_run(argv[1]);
    if(status) {
        printf("\n%[12%s not found%[15",argv[1]);
    }
}

void cmd_dir() {
    printf("\n\n/root:\n");
    uint8* root = malloc(8192);
    get_root(root);
    File file;
    
    while (*root)
    {
        int i = 0;
        int j = 0;
        memcpy(root, (uint8*)&file, 16);
        if(file.name[i] != 0xFF) {
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
        }

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
    uint32 argc = 1;
    uint32 n = 0;
    while (command_buffer[n])
    {
        if(command_buffer[n] == ' ')
            argc++;
        n++;
    }

    argv_buff = (uint32)malloc(argc * 4);
    argv = argv_buff;

    n = 0;
    argc = 1;
    uint8* cmd_head = command_buffer;
    argv[0] = cmd_head;
    while (*cmd_head)
    {
        if(*cmd_head == ' ') {
            *cmd_head = '\0';
            argv[argc] = cmd_head + 1;
            argc++;
        }
        cmd_head++;
    }

    uint32 is_found = 0;
    for(uint32 i = 0;i < COMMAND_COUNT;i++) {
        if(strcmp(argv[0], cmd[i].cmd_name) == 0) {
            cmd[i].handler();
            is_found = 1;
        }
    }

    memset(command_buffer[0], 0, COMMAND_BUFFER_SIZE);

    if(!is_found) {
        printf("\n%[12Atom: Unknown command%[15");
    }

    free(argv_buff);

    command_index = 0;
}

void shell_main() {
    SetBGColor(0);
    SetFGColor(15);
    clear_screen(0);
    printf(info);
    printf("%[10%s/> %[15",user);
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
                printf("\n%[10%s/> %[15",user);
            }else if(c == BACKSPACE && command_index != 0) {
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
                    if(c != CAPS && c != SHIFT && c != BACKSPACE) {
                        command_buffer[command_index++] = c;
                        putchar(c);
                    }
                }
            }
        }
    }
}

void main() {
    command_buffer = malloc(COMMAND_BUFFER_SIZE);

    uint8* user_cfg = malloc(512);
    sys_read("user.cfg", user_cfg);
    uint32 i = 0;
    uint32 j = 0;
    if(user_cfg[i]) {
        while (user_cfg[i] != ':')
            i++;
        i++;
        while (user_cfg[i] != '\n')
        {
            user[j] = user_cfg[i];
            j++;
            i++;
        }
    }

    shell_main();
}
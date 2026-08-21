#include "../libs/strio.h"
#include "../libs/string.h"
#include "../libs/file.h"
#include "../libs/memory.h"
#include "../libs/types.h"

#define COMMAND_BUFFER_SIZE    128
#define COMMAND_COUNT           11

#define SHIFT 0x01
#define CAPS  0x02
#define CTRL  0x03
#define ENTER 0x0A 
#define BACKSPACE 0x08 

typedef struct __attribute__((packed)) {
    U8 name[8];
    U8 ext[3];
    U16 start_sec;
    U16 size;
    U8 flags;
} _File;

typedef struct Command
{
    U8* cmd_name;
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
void cmd_sys();

struct Command cmd[] = {
    {"help", cmd_help},
    {"clear", cmd_clear},
    {"dir", cmd_dir},
    {"run", cmd_run},
    {"exit", sys_died},
    {"cat", cmd_read},
    {"write", cmd_write},
    {"touch", cmd_create},
    {"rm", cmd_delete},
    {"is",cmd_check},
    {"sys",cmd_sys},
};
U32 command_index = 0;
U8 *command_buffer;

U32 *argv_buff;
U8** argv;

/*U8* info =   "\nAtom interactive shell %[13v 0.1%[15\n"
                    "Copyright (c) 2026 Zhoski. Licensed under the MIT License.\n\n"
                    "Type %[14\"help\"%[15 to view the list of commands.\n\n";
*/
U8* info = "%[10    ___   __                      _  ______  _____\n"
           "   /   | / /_____  ____ ___      | |/ / __ \\/ ___/\n"
           "  / /| |/ __/ __ \\/ __ `__ \\_____|   / /_/ / __ \\ \n"
           " / ___ / /_/ /_/ / / / / / /_____/   \\__, / /_/ / \n"
           "/_/  |_\\__/\\____/_/ /_/ /_/     /_/|_/____/\\____/ \n\n\n%[15\n"
           "Copyright (c) 2026 Zhoski. Licensed under the MIT License.\n"
           "Type 'help' to get a list of commands.\n";

U8 user[32];
U8 pass[32];

void cmd_help() {
    U8* help_msg =   "\n"
                        "%[11[BASE]%[15\n"
                        "   clear                -- clear screen\n"
                        "%[11[DISK]%[15\n"
                        "   touch  [file]        -- creates file\n"
                        "   rm     [file]        -- deletes file\n"
                        "   cat    [file]        -- read file\n"
                        "   write  [file] [data] -- write in file\n"
                        "   ls                   -- displays all files in a directory\n"
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
    File* f = fopen(argv[1], FREAD);
    if(!f) {
        printf("%[12Can't open [%s]%[15",argv[1]);
        return;
    }
    if(!f->cur) {
        printf("[File empty]");
    }else {
        U8* buffer = malloc(f->bytes);
        fread(f, f->bytes, buffer);
        printf("%s",buffer);
    }
    fclose(f);
}

void cmd_check() {
    int ret = sys_check(argv[1]);
    if(!ret) {
        printf("\n%[12%s not found%[15", argv[1]);
    }else {
        printf("\n%[10%s found%[15", argv[1]);
    }
}

void cmd_write() {
    int size = 0;
    File* f = fopen(argv[1], FWRITE);
    if(!f) {
        printf("%[12Can't write [%s]%[15",argv[1]);
        return;
    }else {
        U32 s = 0;
        while (argv[2][s++]);
        
        //sys_write(f->name, argv[2], s);
        fwrite(f, s, argv[2]);
    }
    fclose(f);
}

void cmd_delete() {
    int ret = sys_delete(argv[1]);
    if(!ret) {
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
    U32 status = sys_run(argv[1]);
    if(status) {
        printf("\n%[12%s not found%[15",argv[1]);
    }
}

void cmd_dir() {
    printf("\n\n/root:\n");
    U8* root = malloc(8192);
    get_root(root);
    _File file;
    
    while (*root)
    {
        int i = 0;
        int j = 0;
        memcpy(root, (U8*)&file, 16);
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

void cmd_sys() {
    printf("\n");
    U16* ram = (U16*)0x1000;
    U8*  cpu = malloc(48);
    memcpy(0x1006, cpu, 48);

    printf("\n");
    printf("%[10       /\\          %[15%s@Atom-X86\n", user);
    printf("%[10      /  \\         %[15-----------------\n");
    printf("%[10     / /\\ \\        %[10OS:%[15 Atom-X86 Demo\n"); 
    printf("%[10    / /  \\ \\       %[10Kernel:%[15 v0.0.1-custom\n");
    printf("%[10   / /____\\ \\      %[10CPU:%[15 %s\n", cpu); 
    printf("%[10  /__________\\     %[10Memory:%[15 %d\n", ram); 
    printf("%[10 /____________\\    %[10Display:%[15 VGA 640x480 16 colors\n"); 
    printf("                   %[10Shell:%[15 Default\n");
    printf("\n                   %[01\xDB\xDB%[02\xDB\xDB%[03\xDB\xDB%[04\xDB\xDB%[05\xDB\xDB"
        "%[06\xDB\xDB%[07\xDB\xDB%[08\xDB\xDB%[09\xDB\xDB%[10\xDB\xDB%[11\xDB\xDB"
        "%[12\xDB\xDB%[13\xDB\xDB%[14\xDB\xDB%[15\xDB\xDB");
}

void execute() {
    command_buffer[command_index] = '\0';
    if(!command_buffer[0]) {
        command_index = 0;
        return;
    }
    U32 argc = 1;
    U32 n = 0;
    while (command_buffer[n])
    {
        if(command_buffer[n] == ' ')
            argc++;
        n++;
    }

    argv_buff = (U32)malloc(argc * 4);
    argv = argv_buff;

    n = 0;
    argc = 1;
    U8* cmd_head = command_buffer;
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

    U32 is_found = 0;
    for(U32 i = 0;i < COMMAND_COUNT;i++) {
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
    for(U32 i = 0;i < COMMAND_BUFFER_SIZE;i++) {
        command_buffer[i] = 0;
    }

    U8 c = '\0';
    while (1)
    {
        c = get_char();
        if(c) {
            if(c == ENTER) {
                execute();
                printf("\n%[10%s/> %[15",user);
            }else if(c == BACKSPACE && command_index != 0) {
                U32 x;
                U32 y;
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
    
    File* fUser = fopen("user.cfg", FREAD);
    if(!fUser) {
        printf("Error\n");
    }
    U8* user_cfg = malloc(fUser->bytes);
    fread(fUser, fUser->bytes, user_cfg);
    fclose(fUser);

    U32 i = 0;
    U32 j = 0;
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

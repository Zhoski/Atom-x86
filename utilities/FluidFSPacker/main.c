#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "include/request.h"
#include "include/type.h"
#include "include/pack.h"

#define FLAG_MAX    10

request_t request;

typedef struct flag {
    u8* flag;
    void (*handler)(char* argv[], u32 argc, u32* idx);
};

void c_flag(char* argv[], u32 argc, u32* idx) {
    request.flag0 |= 0x01;

    request.disk = argv[*idx + 1];

    (*idx)++;
}

void p_flag(char* argv[], u32 argc, u32* idx) {  
    request.flag0 |= 0x02;

    request.argv = (u8**)&argv[*idx + 1];

    u32 n = 0;

    while((strcmp(argv[*idx + 1], "-o") != 0)) {
        n++;
        (*idx)++;
    }

    request.argc = n;
}

void b_flag(char* argv[], u32 argc, u32* idx) {
    request.flag0 |= 0x04;

    request.boot = argv[*idx + 1];

    (*idx)++;
}

void mb_flag(char* argv[], u32 argc, u32* idx) {
    request.flag0 |= 0x08;

    request.main_boot = argv[*idx + 1];

    (*idx)++; 
}

void K_flag(char* argv[], u32 argc, u32* idx) {
    request.flag0 |= 0x10;

    request.disk_size = (atoi(argv[*idx + 1])) * 1024;

    (*idx)++;

    if(request.disk_size > 256 * 1024 * 1024) {
        printf("[ \033[33mWARN\033[37m ] Maximum disk size: 256 MB\n");
        request.disk_size = 256 * 1024 * 1024;
    }else if(request.disk_size < 64 * 1024) {
        printf("[ \033[33mWARN\033[37m ] Minimum disk size: 64 Kb\n");
        request.disk_size = 64 * 1024;
    }
}

void M_flag(char* argv[], u32 argc, u32* idx) {
    request.flag0 |= 0x20;

    request.disk_size = (atoi(argv[*idx + 1])) * 1024 * 1024;

    (*idx)++;

    if(request.disk_size > 256 * 1024 * 1024) {
        printf("[ WARN ] Maximum disk size: 256 MB\n");
        request.disk_size = 256 * 1024 * 1024;
    }else if(request.disk_size < 32 * 1024) {
        printf("[ WARN ] Minimum disk size: 32 Kb\n");
        request.disk_size = 32 * 1024;
    }
}

void o_flag(char* argv[], u32 argc, u32* idx) {
    request.flag0 |= 0x40;

    request.out_path = argv[*idx + 1];
    (*idx)++;
}

void d_flag(char* argv[], u32 argc, u32* idx) {
    request.flag0 |= 0x100;

    request.disk = argv[*idx + 1];
    (*idx)++;
}

void bs_flag(char* argv[], u32 argc, u32* idx) {
    request.flag0 |= 0x200;

    request.block_size = atoi(argv[*idx + 1]);

    (*idx)++;
}

struct flag f[] = {
    {"-c", c_flag},
    {"-p", p_flag},
    {"-b", b_flag},
    {"-mb", mb_flag},
    {"-K", K_flag},
    {"-M", M_flag},
    {"-o", o_flag},
    {"-d", d_flag},
    {"-f"},
    {"-bs", bs_flag},
};

int main(int argc, char* argv[]) {
    u32 i = 1;
    while (i < argc)
    {
        u8 flag_found = 0;
        for(u32 k = 0; k < FLAG_MAX; k++) {
            if(!(strcmp(f[k].flag, argv[i]))) {
                f[k].handler(argv, argc, &i);
                flag_found = 1;
            }
        }

        if(!flag_found) {
            i++;
        }
    }
    
    pack(&request);
}
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define FALSE       0
#define TRUE        1

#define ROOT_BASE   2 * 512
#define DATA_BASE  18 * 512

#define DISK_SIZE   16 * 1024 * 1024

typedef struct __attribute__((packed)) {
    unsigned char name[8];
    unsigned char ext[3];
    unsigned short start_sec;
    unsigned short size;
    unsigned char flags;
} File;


unsigned char push_flag = FALSE;
unsigned char boot_flag = FALSE;
unsigned char mount_flag = FALSE;
unsigned char create_flag = FALSE;

unsigned char disk[DISK_SIZE];

void afs_make_name(char* file, char* out) {
    char file_name[11];
    int i = 0;
    int j = 0;
    
    const char* filename = strrchr(file, '/');
    
    if (filename != NULL) {
        filename++; 
    } else {
        filename = file; 
    }

    while (filename[j] != '.')
    {
        if(filename[j] >= 97) 
            file_name[i] = filename[j] - 32;
        else 
            file_name[i] = filename[j];

        j++;
        i++;
    }

    while (i < 8)
    {
        file_name[i] = ' ';
        i++;
    }

    while (i < 11)
    {
        j++;
        if(filename[j] >= 97) 
            file_name[i] = filename[j] - 32;
        else 
            file_name[i] = filename[j];

        i++;
    }
    
    for(int n = 0;n < 11;n++) {
        out[n] = file_name[n];
    }
}

unsigned short find_free_sector(char* out) {
    FILE* out_img = fopen(out, "r");
    if(!out_img)
        return 1;

    fread(disk, 1, DISK_SIZE, out_img);
    fclose(out_img);

    unsigned char* ROOT = disk + ROOT_BASE;
    unsigned char* ROOT_HEAD = ROOT;

    File* file = (File*)ROOT_HEAD;

    unsigned short max_start_sec = file->start_sec;
    unsigned short max_size_in_sec = (file->size + 511) >> 9;

    unsigned char flag = FALSE;

    while(*ROOT_HEAD) {
        if(max_start_sec < file->start_sec) {
            max_start_sec = file->start_sec;
            max_size_in_sec = (file->size + 511) >> 9;
        }
        ROOT_HEAD += 16;
        file = (File*)ROOT_HEAD;

        flag = TRUE;
    }

    if(flag) {
        return max_start_sec + max_size_in_sec;
    }
    return 18;
}

int push(char* out, char* __file) {
    printf("afms: Push %s to %s\n", __file, out);
    FILE* file_data = fopen(__file, "r");
    if(!file_data)
        return 1;

    fseek(file_data, 0, SEEK_END);
    unsigned short size = ftell(file_data);

    fseek(file_data, 0, SEEK_SET);
    unsigned char* file_data_buffer = (unsigned char*)malloc(size);
    fread(file_data_buffer, 1, size, file_data);

    fclose(file_data);

    char file_name[11];
    afs_make_name(__file, file_name);

    File file;
    memcpy((unsigned char*)&file, file_name, 11);
    
    file.size = size;
    file.start_sec = find_free_sector(out);
    file.flags = 0x1;

    FILE* out_disk = fopen(out, "r+");
    if(!out_disk)
        return 1;

    fread(disk, 1, DISK_SIZE, out_disk);
    unsigned char* disk_head = disk;
    disk_head += ROOT_BASE;

    while (*disk_head)
    {
        disk_head += 16;
    }

    memcpy(disk_head, (unsigned char*)&file, 16);
    
    disk_head = disk + (file.start_sec * 512);
    memcpy(disk_head, file_data_buffer, size);

    fseek(out_disk, 0, SEEK_SET);
    fwrite(disk, 1, DISK_SIZE, out_disk);
    
    fclose(out_disk);
}

int boot(char* out, char* boot_path) {
    printf("afms: Create MBR from %s in %s\n", boot_path, out);
    FILE* boot_entry = fopen(boot_path, "r");
    if(!boot_entry) 
        return 1;

    unsigned char boot_buff[512];
    fread(boot_buff, 1 , 512, boot_entry);

    fclose(boot_entry);

    memcpy(disk, boot_buff, 512);

    FILE* out_img = fopen(out, "w");
    if(!out_img) 
        return 1;

    fwrite(disk, 1, DISK_SIZE, out_img);

    fclose(out_img);
}

int create(char* out) {
    printf("afms: Create %s, size 16M\n", out);
    FILE* out_img = fopen(out, "w");
    if(!out_img) 
        return 1;

    fwrite(disk, 1, DISK_SIZE, out_img);
    fclose(out_img);
}

int main(int argc, char* argv[]) {
    if(!argv) {
        return -1;
    }
    
    for(int i = 0;i < argc;i++) {
        if(strcmp(argv[i], "-push") == 0) {
            push_flag = TRUE;
            push(argv[2], argv[3]);
        }
        if(strcmp(argv[i], "-boot") == 0) {
            boot_flag = TRUE;
            boot(argv[2], argv[3]);
        }
        if(strcmp(argv[i], "-c") == 0) {
            create_flag = TRUE;
            create(argv[2]);
        }
        if(strcmp(argv[i], "-mount") == 0) {
            mount_flag = TRUE;
        }
    }
}
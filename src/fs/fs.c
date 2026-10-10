#include <fs/fs.h>
#include <fs/afs.h>
#include <fs/fluidfs.h>
#include <drivers/video/video.h>

static FileSystem_t AFS = {
    .init = &afs_init,
    .open = &afs_open,
    .read = &afs_read,
    .create = &afs_create,
    .delete = &afs_delete,
    .update = &afs_update,
    .get_root = &afs_get_root,
    .check = &afs_check_file,
};

static FileSystem_t FluidFS = {
    .init = &fluidfs_init,
    .check = &fluidfs_fcheck,
    .open = &fluidfs_fopen,
};

FileSystem_t* fs;

u32 init_fs(u32 type) {
    if(type == AFS_T) {
        fs = &AFS;
    }else if(type == FLUIDFS_T) {
        fs = &FluidFS;
    }

    fs->init();
}
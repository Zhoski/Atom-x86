#include <fs/fs.h>
#include <fs/fluidfs.h>
#include <drivers/video/video.h>

static FileSystem_t FluidFS = {
    .init = &fluidfs_init,
    .check = &fluidfs_fcheck,
    .open = &fluidfs_fopen,
    .read = &fluidfs_fread,
    .update = &fluidfs_fwrite,
    .delete = &fluidfs_fdel,
    .create = &fluidfs_cnode,
    .write_dir = &fluidfs_write_dir
};

FileSystem_t* fs;

u32 init_fs(u32 type) {
    fs = &FluidFS;

    fs->init();
}
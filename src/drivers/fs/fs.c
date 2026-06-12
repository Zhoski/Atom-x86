#include "fs.h"
#include "afs.h"

static FileSystem AFS = {
    .init = &afs_init,
    .open = &afs_open,
    .read = &afs_read,
    .create = &afs_create,
    .delete = &afs_delete,
    .update = &afs_update,
};

FileSystem* fs;

void init_fs() {
    fs = &AFS;
    fs->init();
}
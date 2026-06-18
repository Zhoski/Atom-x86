#include "fs.h"
#include "afs.h"

static FileSystem AFS = {
    .init = &afs_init,
    .open = &afs_open,
    .read = &afs_read,
    .create = &afs_create,
    .delete = &afs_delete,
    .update = &afs_update,
    .get_root = &afs_get_root,
};

FileSystem* fs;

void init_fs() {
    fs = &AFS;
    fs->init();
}
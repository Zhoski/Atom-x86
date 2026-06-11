#include "fs.h"
#include "afs.h"

static FileSystem AFS = {
    .open = &afs_open,
    .read = &afs_read,
    .create = &afs_create,
    .delete = &afs_delete,
};

FileSystem* fs;

void init_fs() {
    fs = &AFS;
}
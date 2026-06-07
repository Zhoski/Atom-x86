#include "fs.h"
#include "afs.h"

static FileSystem AFS = {
    .open = &afs_open,
};

FileSystem* fs;

void init_fs() {
    fs = &AFS;
}
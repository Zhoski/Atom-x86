#include "../../libs/file.h"
#include "../../libs/strio.h"

void main() {
    if(sys_check("USER    CFG")) {
        sys_run("SETUP   BIN");
    }else {
        sys_run("SHELL   BIN");
    }
}
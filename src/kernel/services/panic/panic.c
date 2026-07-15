#include "panic.h"

void kernel_panic() {
    asm("cli");
    for(;;) {
        asm("hlt");
    }
}
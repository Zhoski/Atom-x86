#include <kernel/panic.h>
#include <drivers/video/video.h>

void kernel_panic() {
    video->clear_screen(1);
    video->write_string("PANIC");
    asm("cli");
    for(;;) {
        asm("hlt");
    }
}
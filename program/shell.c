void _start() {
    static const char msg[] __attribute__((section(".text"))) = "Hello World";
    asm("movl $1, %%eax\n"
        "movl $1, %%ebx\n"
        "movl %0, %%ecx\n"
        "int $0x80"
            :
            : "r" (msg)
            : "%eax", "%ebx", "%ecx"
        );

    for(;;) {
        asm("hlt");
    }
}

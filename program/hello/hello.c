#include "../libs/strio.h"

void main() {
    clear_screen(1);
    printf("Maksim pakostnik\n");
    printf("Press any key to return");
    uint8 c = get_char();
    while (!c)
    {
        c = get_char();
    }
    

    asm volatile (
        "movl $60, %%eax\n"
        "int $0x80"
        :
        :
        : "eax"
    );
}
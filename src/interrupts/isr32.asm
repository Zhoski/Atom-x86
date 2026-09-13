extern timer_handler
global isr32

isr32:
    cli

    pushad
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov es, ax
    mov ds, ax

    call timer_handler

    mov al, 0x20
    out 0x20, al
    out 0xA0, al

    pop gs
    pop fs
    pop es
    pop ds
    popad

    iret

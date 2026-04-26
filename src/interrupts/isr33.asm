extern keyboard_handler

global isr33

isr33:
    cli

    pushad
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov es, ax
    mov ds, ax

    call keyboard_handler

    mov al, 0x20
    out 0x20, al

    pop gs
    pop fs
    pop es
    pop ds
    popad

    iret

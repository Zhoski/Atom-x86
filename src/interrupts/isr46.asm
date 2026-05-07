extern disk_handler

global isr46

isr46:
    cli

    pushad
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov es, ax
    mov ds, ax

    call disk_handler

    mov al, 0x20
    out 0x20, al
    out 0xA0, al

    pop gs
    pop fs
    pop es
    pop ds
    popad

    iret

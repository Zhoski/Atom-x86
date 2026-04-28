extern syscall_handler
global isr80

isr80: 
    cli

    pushad
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov es, ax
    mov ds, ax
    
    push edx
    push ecx
    push ebx
    push eax

    call syscall_handler

    add esp, 16

    pop gs
    pop fs
    pop es
    pop ds
    popad
    
    sti
    iret

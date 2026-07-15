extern kernel_panic

isr8:
    cli

    mov eax, 0
    mov edi, 0x90000
    mov ecx, 0x10000
    rep stosb

    mov ebp, 0x100000
    mov esp, ebp

.stop:
    hlt
    jmp .stop
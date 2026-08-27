extern kernel_panic

isr8:
    cli

    call kernel_panic

.stop:
    hlt
    jmp .stop
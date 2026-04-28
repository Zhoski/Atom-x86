bits 32

global shell
shell:
    mov word [0xB8000], 0x4F41  ; 41 = 'A', 4F
    
    jmp $

    ret

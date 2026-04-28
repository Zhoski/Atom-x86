bits 32

global shell
shell:
    mov word [0xB8000], 0x4F41  ; 41 = 'A', 4F

    mov eax, 0    ; Номер системного вызова
    mov ebx, 0    ; Аргумент 1
    mov ecx, 0    ; Аргумент 2
    mov edx, 0    ; Аргумент 3
    
    int 0x80 
    jmp $

    ret

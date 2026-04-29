bits 32

global shell
shell: 
    mov eax, 1    ; Номер системного вызова
    mov ebx, 'H'  ; Аргумент 1
    mov ecx, 0    ; Аргумент 2
    mov edx, 0    ; Аргумент 3
    
    int 0x80 
    jmp $

    ret



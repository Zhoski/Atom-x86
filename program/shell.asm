bits 32
org 0x200000
global shell
shell: 
    ; ebx 1 = Вывести текст
    ; ebx 2 = Вывести переменную

    mov eax, 1    ; Вывод текста
    mov ebx, 1    ; Формат вывода (текс)
    mov ecx, msg  ; Что выводить
    mov edx, 0    ; Аргумент 3
    
    int 0x80 

    jmp loop

    jmp $

    ret

loop:
    mov eax, 2
    mov ebx, 1
    int 0x80
    mov ecx, eax
    mov eax, 1
    mov ebx, 3
    int 0x80
    jmp loop

print_string:
    

msg: db "--- Shell version 0.1 ---",0
